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

#include <log.h>
#include <horus_key.h>
#include <openssl/sha.h>

struct _key_store_entry *key_store = NULL, *key_store_tail = NULL;

void
key_to_value (char *key, int *key_len, char *string)
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
key_to_string (char *buf, int size, char *key, int key_len)
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
  key_to_string (buf, sizeof (buf), key, key_len);
  return buf;
}

char *
block_key (char *key, size_t *key_len,
           char *parent, int parent_len, int x, int y)
{
  void *message;
  int size;

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

  if (debug)
    {
      printf ("parent = %s\n", print_key (parent, parent_len));
      printf ("x: %d, y: %d, message: %s, size: %d\n", x, y, x__y, size);
      printf ("message = %s\n", print_key (x__y, size));
    }

#ifdef __APPLE__
  CCHmac (kCCHmacAlgSHA1, parent, parent_len,
          (const void *)message, size, key);
  *key_len = SHA_DIGEST_LENGTH;
#else /*__APPLE__*/
  HMAC (EVP_sha1(), parent, parent_len,
        (const unsigned char *) message, size,
        (unsigned char *) key, key_len);
#endif /*__APPLE__*/

  if (debug)
    printf ("key = %s\n", print_key (key, *key_len));

  return key;
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

  block_key (out_key, &key_len, parent_key, HORUS_KEY_LEN, x, y);
  vectorx_set (key_vec_x, y, duplicate_key (out_key, HORUS_KEY_LEN));
  
  return 0;
}
