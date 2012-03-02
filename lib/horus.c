/*
 * Horus Functions
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

#include "horus_key.h"
#include "horus_crypt.h"
#include "horus.h"
#include "log.h"

int debug = 0;
int is_init = 0;

/* libhorus tracks files by file descriptor. */
fd_set fdset;

void
horus_init ()
{
  FD_ZERO (&fdset);
  is_init++;
}

int
horus_open (int fd, const char *path, int oflag, mode_t mode)
{
  char *p, *filename, *match;

  if (! is_init)
    horus_init ();

  if (fd < 0)
    return -1;

  match = getenv ("HORUS_MATCH_FILE");
  if (! match)
    return 0;

  p = rindex (path, '/');
  filename = (p ? ++p : (char *) path);

  if (! strcasecmp (filename, match))
    FD_SET (fd, &fdset);

  return 0;
}

extern struct _key_store_entry *key_store, *key_store_tail;


/** Scans key_store to find the entry for fd, create a new entry if not found
 */
struct
_key_store_entry *find_key_by_fd_or_create_new (const int fd)
{
  struct _key_store_entry *p = find_key_by_fd (fd);

  if (NULL != p)
    return p;
  else
    {
      // not found, allocate a new entry
      p = malloc (sizeof (struct _key_store_entry));
      if (NULL == p)
	abort ();
      memset (p, 0, sizeof (struct _key_store_entry));
      p->fd = fd;
      p->cipher = aes_xts_init();
      // first entry of key_store?
      if (NULL == key_store)
	{
	  key_store = p;
	  key_store_tail = p;
	}
      else
	{
	  // append new_entry to tail
	  key_store_tail->next = p;
	  key_store_tail = p;
	}
    }

  return p;
}

ssize_t
horus_pread (int fd, void *buf, size_t size, off_t fdpos)
{
  perror (__func__);
  abort ();
  return 0;
}

ssize_t
horus_pwrite (int fd, void *buf, size_t size, off_t fdpos)
{
  perror (__func__);
  abort ();
  return 0;
}

ssize_t
horus_read (int fd, void *buf, size_t nbyte)
{
  void *encrypted_buf = NULL;
  void *horus_block_key = NULL;
  u8 iv[HORUS_KEY_LEN];
  size_t nbyte_remain = nbyte;
  size_t block_size;
  size_t block_id;

  struct _key_store_entry *key_info = find_key_by_fd (fd);
  if (NULL == key_info)
    {
      log_error ("file hasn't been properly inited by horus");
      abort ();
    }
  block_size = key_info->leaf_block_size;

  size_t fdpos = lseek (fd, 0, SEEK_CUR);
  if ( (fdpos % block_size != 0) ||
       (nbyte % block_size != 0) )
    {
      log_error ("Read on non-aligned block is not supported yet");
      abort ();
    }

  encrypted_buf = malloc (block_size);
  if (NULL == encrypted_buf)
    {
      perror (__func__);
      abort ();
    }
  while (nbyte_remain > 0)
    {
      block_id = fdpos / block_size;
      horus_get_leaf_block_key (fd, &horus_block_key, block_id);
      aes_xts_setkey (key_info->cipher, horus_block_key, HORUS_KEY_LEN);
      // Use block_id as IV
      memset (iv, 0, sizeof (iv));
      *(size_t *)iv = block_id;

      if ( block_size !=
	   read (fd, encrypted_buf, block_size) )
	{
	  perror (__func__);
	  abort ();
	}
      aes_xts_decrypt (key_info->cipher,
		       buf + (nbyte - nbyte_remain),
		       encrypted_buf,
		       block_size,
		       iv);
      fdpos += block_size;
      nbyte_remain -= block_size;
    }

  if (encrypted_buf)
    free (encrypted_buf);
  return nbyte;
}

ssize_t
horus_write (int fd, void *buf, size_t nbyte)
{
  void *encrypted_buf = NULL;
  void *horus_block_key = NULL;
  u8 iv[HORUS_KEY_LEN];
  size_t nbyte_remain = nbyte;
  size_t block_size;
  size_t block_id;

  struct _key_store_entry *key_info = find_key_by_fd (fd);
  if (NULL == key_info)
    {
      log_error ("file hasn't been properly inited by horus");
      abort ();
    }
  block_size = key_info->leaf_block_size;

  size_t fdpos = lseek (fd, 0, SEEK_CUR);
  if ( (fdpos % block_size != 0) ||
       (nbyte % block_size != 0) )
    {
      log_error ("Write on non-aligned block is not supported yet");
      abort ();
    }

  encrypted_buf = malloc (block_size);
  if (NULL == encrypted_buf)
    {
      perror (__func__);
      abort ();
    }
  while (nbyte_remain > 0)
    {
      block_id = fdpos / block_size;
      horus_get_leaf_block_key (fd, &horus_block_key, block_id);
      aes_xts_setkey (key_info->cipher, horus_block_key, HORUS_KEY_LEN);
      // Use block_id as IV
      memset (iv, 0, sizeof (iv));
      *(size_t *)iv = block_id;

      aes_xts_encrypt (key_info->cipher,
		       encrypted_buf,
		       buf + (nbyte - nbyte_remain),
		       block_size,
		       iv);
      if ( block_size !=
	   write (fd, encrypted_buf, block_size) )
	{
	  perror (__func__);
	  abort ();
	}
      fdpos += block_size;
      nbyte_remain -= block_size;
    }

  if (encrypted_buf)
    free (encrypted_buf);
  return nbyte;
}

int
horus_close (int fd)
{
  if (! is_init)
    horus_init ();

  if (fd < 0)
    return -1;

  if (FD_ISSET (fd, &fdset))
    FD_CLR (fd, &fdset);

  return 0;
}


void
horus_set_kht (const int fd, int depth, size_t leaf_block_size, const int *branching_factor)
{
  int i;

  struct _key_store_entry *p = find_key_by_fd_or_create_new (fd);
  assert (fd == p->fd);

  p->depth = depth;
  p->leaf_block_size = leaf_block_size;
  // TODO: free old key_pool
  p->key_vec = malloc (sizeof (*p->key_vec) * depth);
  for (i = 0; i < depth; ++i)
      p->key_vec[i] = vectorx_create ();
  if (p->branching_factor)
    free (p->branching_factor);
  size_t branching_factor_size = sizeof (*branching_factor) * (depth - 1);
  p->branching_factor = malloc (branching_factor_size);
  if (NULL == p->branching_factor)
    abort (); // TODO
  memcpy (p->branching_factor, branching_factor, branching_factor_size);
}

int
horus_add_key (const int fd,
	       const void *key, size_t key_len,
	       const int x, const int y)
{
  void *key_to_add;
  void *old_key;
  struct vectorx *key_vec_x;
  struct _key_store_entry *p = find_key_by_fd (fd);

  if (NULL == p)
    {
      perror (__func__);
      abort ();
    }

  if (x > p->depth)
    {
      perror ("Trying to set key at a level deeper than the depth of KHT.");
      abort ();
    }

  key_vec_x = p->key_vec[x-1];
  if (y < key_vec_x->limit)
    {
      if ((old_key = vectorx_get (key_vec_x, y)) != NULL)
	free (old_key);
    }

  key_to_add = malloc (HORUS_KEY_LEN);
  if (NULL == key_to_add)
    abort ();
  memset (key_to_add, 0, HORUS_KEY_LEN);
  memcpy (key_to_add, key, key_len > HORUS_KEY_LEN ? HORUS_KEY_LEN : key_len);
  vectorx_set (key_vec_x, y, key_to_add);

  return 0;
}

int
horus_get_leaf_block_key (const int fd, void **out_key, size_t block_id)
{
  struct _key_store_entry *p = find_key_by_fd (fd);

  if (NULL == p)
    {
      perror (__func__);
      abort ();
    }
  
  return horus_get_key (fd, out_key, p->depth, block_id);
}
