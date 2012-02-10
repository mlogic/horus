/*
 * Horus AES Cipher. Based on Linux kernel 3.2.3.
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 */

#ifndef _XTS_H
#define _XTS_H

#include "crypto.h"

struct aes_xts_cipher {
  struct aes_cipher *child;
  struct aes_cipher *tweak;
};

struct aes_xts_cipher *aes_xts_init();

int aes_xts_setkey(struct aes_xts_cipher *ctx, const u8 *key,
		   unsigned int keylen);
void aes_xts_encrypt(struct aes_xts_cipher *ctx, void *dst,
		     const void *src, size_t nbytes, void *iv);
void aes_xts_decrypt(struct aes_xts_cipher *ctx, void *dst,
		     const void *src, size_t nbytes, void *iv);

#endif /* _XTS_H */
