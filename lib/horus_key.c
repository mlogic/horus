/*
 * Horus Key Utils
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yasuhiro Ohara <yasu@soe.ucsc.edu>
 *  Yan Li <yanli@ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 */

#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>
#include <log.h>

struct _key_store_entry *key_store = NULL, *key_store_tail = NULL;

void
key_str2val (char *key, int *key_len, char *string)
{
  char *p;
  char c[2];
  unsigned char val[2];
  int index = 0;
  int len = 0;

  for (p = string; *p != '\0'; p++)
    {
      c[index] = *p;
      if ('0' <= c[index] && c[index] <= '9')
        val[index] = c[index] - '0';
      else if ('a' <= c[index] && c[index] <='f')
        val[index] = c[index] - 'a' + 10;
      else if ('A' <= c[index] && c[index] <='F')
        val[index] = c[index] - 'A' + 10;
      else
        val[index] = 0;

      if (index == 1)
        key[len++] = (unsigned char) ((val[0] << 4) + val[1]);

      index ^= 1;
    }

  if (index == 1)
    {
      syslog (LOG_WARNING, "invalid key length: %d: %s",
              (int) strlen (string), string);
      key[len++] = (unsigned char) (val[0] << 4);
    }

  *key_len = len;
}

void
key_val2str (char *buf, int size, char *key, int key_len)
{
  int i;
  unsigned char val0, val1;
  int len;

  len = 0;
  for (i = 0; i < key_len; i++)
    {
      if (size - len <= 1)
        break;

      val0 = ((unsigned char) key[i] & 0xf0) >> 4;
      if (val0 < 10)
        buf[len] = '0' + val0;
      else
        buf[len] = 'a' + (val0 - 10);
      len++;

      if (size - len <= 1)
        break;

      val1 = (unsigned char) key[i] & 0x0f;
      if (val1 < 10)
        buf[len] = '0' + val1;
      else
        buf[len] = 'a' + (val1 - 10);
      len++;
    }

  if (size - len > 0)
    buf[len] = '\0';
}

char *
print_key (char *key, int key_len)
{
  static char buf[SHA_DIGEST_LENGTH * 2 + 1];
  key_val2str (buf, sizeof (buf), key, key_len);
  return buf;
}

/* K_{x,y} = KH(K_{parent},x||y) */
char *
block_key (char *key, size_t *key_len,
           char *parent, int parent_len, int x, int y)
{
  void *message;
  int size;
  unsigned int out_key_len;

  /* The message x||y */
  assert (x < 256);
  assert (y < 65536 * 256);
  /* Currently we are using string messages for SHA1. */
#undef NON_STRING_MESSAGE
#ifdef NON_STRING_MESSAGE
  u_int32_t x__y = ((x << 24) | y);
  message = &x__y;
  size = sizeof (x__y);
#else /*NON_STRING_MESSAGE*/
  char x__y[17];
  message = x__y;
  size = sizeof (x__y) - 1;
  snprintf (x__y, sizeof (x__y), "%016x", ((x << 24) | y));
#endif /*NON_STRING_MESSAGE*/

#ifdef __APPLE__
  CCHmac (kCCHmacAlgSHA1, parent, parent_len,
          (const void *)message, size, key);
  *key_len = SHA_DIGEST_LENGTH;
#else /*__APPLE__*/
  /* HMAC outputs a uint to out_key_len, need to convert it to size_t
     for key_len */
  HMAC (EVP_sha1(), parent, parent_len,
        (const unsigned char *) message, size,
        (unsigned char *) key, &out_key_len);
  *key_len = (size_t)out_key_len;
#endif /*__APPLE__*/

  if (debug || horus_debug)
    {
      printf ("parent = %s\n", print_key (parent, parent_len));
      printf ("x: %d, y: %d, message: %s, size: %d\n", x, y, x__y, size);
      printf ("message = %s\n", print_key (x__y, size));
      printf ("key = %s\n", print_key (key, *key_len));
    }

  return key;
}

/* Non-recursive version of horus_key_from_to() */
int
horus_block_key (char *k, size_t *klen, int kx, int ky,
                 char *kp, size_t kplen, int kpx, int kpy,
                 unsigned int *kht_block_size)
{
  int i;
  int x, y;
  int branch;
  int xstack[HORUS_MAX_KHT_DEPTH];
  int ystack[HORUS_MAX_KHT_DEPTH];

  char tmp_k[SHA_DIGEST_LENGTH * 2 + 1];
  char tmp_kp[SHA_DIGEST_LENGTH * 2 + 1];
  size_t tmp_klen, tmp_kplen;

  char str_k[SHA_DIGEST_LENGTH * 2 + 1];

  if (horus_debug)
    printf ("%s(): kx = %d, ky = %d, kpx = %d, kpy = %d\n",
            __func__, kx, ky, kpx, kpy);

  assert (kx < HORUS_MAX_KHT_DEPTH);
  assert (kpx < HORUS_MAX_KHT_DEPTH);
  assert (kpx < kx);

  memset (xstack, 0, sizeof (xstack));
  memset (ystack, 0, sizeof (ystack));

  memcpy (tmp_kp, kp, MIN (sizeof (tmp_kp), kplen));
  tmp_kplen = kplen;

  /* Go up KHT remembering the x,y in the stack */
  i = 0;
  x = kx;
  y = ky;
  while (x > kpx)
    {
      assert (i < HORUS_MAX_KHT_DEPTH);
      xstack[i] = x;
      ystack[i] = y;
      i++;

      branch = kht_block_size[x - 1] / kht_block_size[x];
      x -= 1;
      y /= branch;
    }

  assert (x == kpx);
  assert (y == kpy); // Otherwise, outside of the parent range.

  /* Go down KHT back to the requested key */
  while (i > 0)
    {
      i--;
      x = xstack[i];
      y = ystack[i];

      /* K_x,y = KH(Kp, x||y) */
      block_key (tmp_k, &tmp_klen, tmp_kp, tmp_kplen, x, y);

      /* Kp = K_x,y */
      memcpy (tmp_kp, tmp_k, sizeof (tmp_k));
      tmp_kplen = tmp_klen;

      /* Kp = str(Kp) */
      key_val2str (str_k, sizeof (str_k), tmp_kp, tmp_kplen);
      memcpy (tmp_kp, str_k, sizeof (tmp_kp));
      tmp_kplen = strlen (str_k);

      if (horus_verbose)
        printf ("K_%d,%d = %s\n", x, y, tmp_kp);
    }

  assert (x == kx && y == ky);

  /* k = tmp_k */
  memcpy (k, tmp_k, *klen);
  *klen = tmp_klen;

  return 0;
}

int
horus_key_by_master (char *key, size_t *key_len, int x, int y,
                     char *master, int master_len,
                     unsigned int *kht_block_size)
{
  char k0[SHA_DIGEST_LENGTH * 2 + 1];
  size_t k0len;
  int xk0, yk0;
  char str_k0[SHA_DIGEST_LENGTH * 2 + 1];
  int tk, ty;
  int branch;

  if (horus_debug)
    printf ("%s(): x = %d, y = %d\n", __func__, x, y);

  tk = x;
  ty = y;
  while (tk > 0)
    {
      branch = kht_block_size[tk - 1] / kht_block_size[tk];
      tk -= 1;
      ty /= branch;
    }

  xk0 = tk;
  yk0 = ty;
  assert (xk0 == 0);

  block_key (k0, &k0len, master, (size_t) master_len, xk0, yk0);

  key_val2str (str_k0, sizeof (str_k0), k0, k0len);
  memcpy (k0, str_k0, sizeof (k0));
  k0len = strlen (str_k0);

  if (xk0 == x)
    {
      memcpy (key, k0, sizeof (key));
      *key_len = k0len;
    }
  else
    {
      horus_block_key (key, key_len, x, y, k0, k0len, xk0, yk0,
                       kht_block_size);
    }

  return 0;
}



/** Scan key_store and look for entry of fd
 */
struct _key_store_entry*
find_key_by_fd (const int fd)
{
  struct _key_store_entry *p = key_store;
  while (p != NULL)
    {
      if (fd == p->fd)
	return p;
    }

  // not found
  return NULL;
}

int
horus_get_key (const int fd, void *out_key, const int x, const int y)
{
  struct vectorx *key_vec_x;
  u8 parent_key[HORUS_KEY_LEN];
  void *my_key;
  struct _key_store_entry *p = find_key_by_fd (fd);

  // sanity check
  if (NULL == p)
    {
      perror (__func__);
      abort ();
    }
  if (x > p->depth)
    {
      perror ("Trying to get a key at a level deeper than the depth of KHT.");
      abort ();
    }

  // handling master key
  if (x == 0)
    {
      if (p->master_key)
	{
	  memcpy (out_key, p->master_key, HORUS_KEY_LEN);
	  return 0;
	}
      else
	return -1;
    }

  // check if the key exists
  key_vec_x = p->key_vec[x-1];
  if ((my_key = vectorx_get (key_vec_x, y)) != NULL)
    {
      // we already have the key, return it
      memcpy (out_key, my_key, HORUS_KEY_LEN);
      return 0;
    }

  // no key exists, we need to calculate it now
  size_t key_len;
  int parent_x, parent_y;
  if (x == 1)
    {
      parent_x = 0;
      parent_y = 0;
    }
  else
    {
      int parent_branching_factor = p->branching_factor[x-2];
      parent_x = x - 1;
      parent_y = y / parent_branching_factor;
    }
  // get parent key and compute K(x,y)
  if (0 != horus_get_key (fd, parent_key, parent_x, parent_y))
      return -1;

  block_key (out_key, &key_len, (char*)parent_key, HORUS_KEY_LEN, x, y);
  vectorx_set (key_vec_x, y, duplicate_key (out_key, HORUS_KEY_LEN));
  
  return 0;
}
