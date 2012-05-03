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

#ifndef _HORUS_KEY_H_
#define _HORUS_KEY_H_

void key_str2val (char *key, int *key_len, char *string);
void key_val2str (char *buf, int size, char *key, int key_len);
char *print_key (char *key, int key_len);
char *block_key (char *key, size_t *key_len,
		 char *parent, int parent_len, int x, int y);

int
horus_block_key (char *k, size_t *klen, int kx, int ky,
                 char *kp, size_t kplen, int kpx, int kpy,
                 unsigned int *kht_block_size);
int
horus_key_by_master (char *key, size_t *key_len, int x, int y,
                     char *master, int master_len,
                     unsigned int *kht_block_size);

int
horus_key_y_of (int level, int ax, int ay, unsigned int *kht_block_size);

#endif /*_HORUS_KEY_H_*/

