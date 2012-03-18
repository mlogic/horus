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

#ifndef _HORUS_KEY_H
#define _HORUS_KEY_H

#include <stdlib.h>
#include "vectorx.h"
#include "xts.h"

struct _key_store_entry
{
  int fd;
  int depth;
  void *master_key;
  size_t leaf_block_size;
  struct vectorx **key_vec;
  int *branching_factor;
  size_t *block_size;

  struct aes_xts_cipher *cipher;

  struct _key_store_entry *next;
};

void key_str2val (char *key, int *key_len, char *string);
void key_val2str (char *buf, int size, char *key, int key_len);
char *print_key (char *key, int key_len);
char *block_key (char *key, size_t *key_len,
		 char *parent, int parent_len, int x, int y);
void *duplicate_key (const void *key, size_t key_len);

struct _key_store_entry* find_key_by_fd (const int fd);

#endif /* _HORUS_KEY_H */
