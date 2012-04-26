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
  else
    buf[size - 1] = '\0';
}

char *
print_key (char *key, int key_len)
{
  static char buf[128];
  memset (buf, 0, sizeof (buf));
  key_val2str (buf, sizeof (buf), key, key_len);
  return buf;
}

/* K_{x,y} = KH(K_{parent},x||y) */
char *
block_key (char *key, size_t *key_len,
           char *pkey, int pkey_len, int x, int y)
{
  void *message;
  int size;
  unsigned int out_key_len;
  char *parent = pkey;
  int parent_len = pkey_len;

  /* The message x||y */
  assert (x < 256);
  assert (y < 65536 * 256);
  /* Currently we are using string messages for SHA1. */
#ifdef STRING_MESSAGE
  char x__y[17];
  message = x__y;
  size = sizeof (x__y) - 1;
  snprintf (x__y, sizeof (x__y), "%016x", ((x << 24) | y));
#else /*STRING_MESSAGE*/
  u_int32_t x__y = ((x << 24) | y);
  message = &x__y;
  size = sizeof (x__y);
#endif /*STRING_MESSAGE*/

#ifdef __APPLE__
  CCHmac (kCCHmacAlgSHA1, parent, parent_len,
          (const void *)message, size, key);
  out_key_len = SHA_DIGEST_LENGTH;
  *key_len = (size_t)out_key_len;
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
      printf ("%s: K_p = 0x%s (len: %d)\n",
              __func__, print_key (parent, parent_len), parent_len);
#ifdef STRING_MESSAGE
      printf ("%s: message(x||y): (%d||%d): \"%s\" (0x%s) (len: %d)\n",
              __func__, x, y, x__y, print_key (x__y, size), size);
#else /*STRING_MESSAGE*/
      printf ("%s: message(x||y): (%d||%d): %#010x (len: %d)\n",
              __func__, x, y, x__y, size);
#endif /*STRING_MESSAGE*/
      printf ("%s: K_%d,%d = 0x%s (len: %d)\n",
              __func__, x, y, print_key (key, *key_len), (int) *key_len);
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
#ifdef PARENT_KEY_STRING_CONVERSION
  char str_kp[SHA_DIGEST_LENGTH * 2 + 1];
#endif /*PARENT_KEY_STRING_CONVERSION*/

  char tmp_k[SHA_DIGEST_LENGTH * 2 + 1];
  char tmp_kp[SHA_DIGEST_LENGTH * 2 + 1];
  size_t tmp_klen, tmp_kplen;

  if (horus_debug)
    printf ("%s: kx = %d, ky = %d, kpx = %d, kpy = %d\n",
            __func__, kx, ky, kpx, kpy);

  assert (kx < HORUS_MAX_KHT_DEPTH);
  assert (kpx < HORUS_MAX_KHT_DEPTH);
  assert (kpx < kx);

  memset (xstack, 0, sizeof (xstack));
  memset (ystack, 0, sizeof (ystack));

  /* Go up KHT recording each (x,y)s in the stack */
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

  /* init Kp */
  memcpy (tmp_kp, kp, MIN (sizeof (tmp_kp), kplen));
  tmp_kplen = kplen;

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

#ifdef PARENT_KEY_STRING_CONVERSION
      /* Kp = str(Kp) */
      key_val2str (str_kp, sizeof (str_kp), tmp_kp, tmp_kplen);
      memcpy (tmp_kp, str_kp, sizeof (tmp_kp));
      tmp_kplen = strlen (str_kp);
#endif
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
  int k0x, k0y;
  int branch;
#ifdef PARENT_KEY_STRING_CONVERSION
  char str_kp[SHA_DIGEST_LENGTH * 2 + 1];
#endif /*PARENT_KEY_STRING_CONVERSION*/

  if (horus_debug)
    printf ("%s: x = %d, y = %d\n", __func__, x, y);

  if (x >= HORUS_MAX_KHT_DEPTH)
    {
      printf ("%s: x: out of range: %d\n", __func__, x);
      return -1;
    }

  if (kht_block_size[x] == 0)
    {
      printf ("%s: zero block size level: x: %d\n", __func__, x);
      return -1;
    }

  k0x = x;
  k0y = y;
  while (k0x > 0)
    {
      branch = kht_block_size[k0x - 1] / kht_block_size[k0x];
      k0x --;
      k0y /= branch;
    }
  assert (k0x == 0);

  block_key (k0, &k0len, master, (size_t) master_len, k0x, k0y);

  if (horus_debug)
    printf ("%s: K_0,0 (len: %d) = %s\n",
            __func__, (int) k0len, print_key (k0, k0len));

  if (k0x == x)
    {
      memcpy (key, k0, *key_len);
      *key_len = k0len;
    }
  else
    {
#ifdef PARENT_KEY_STRING_CONVERSION
      /* Kp = str(Kp) */
      key_val2str (str_kp, sizeof (str_kp), k0, k0len);
      memcpy (k0, str_kp, sizeof (k0));
      k0len = strlen (str_kp);
#endif /*PARENT_KEY_STRING_CONVERSION*/

      horus_block_key (key, key_len, x, y, k0, k0len, k0x, k0y,
                       kht_block_size);
    }

  return 0;
}

