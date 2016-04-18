/*
 * Horus AES Cipher using Intel AES-NI, glue code
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ascar.io>
 *
 * Based on Linux kernel 3.2.3:
 *
 * Support for Intel AES-NI instructions. This file contains glue
 * code, the real AES implementation is in intel-aes_asm.S.
 *
 * Copyright (C) 2008, Intel Corp.
 *    Author: Huang Ying <ying.huang@intel.com>
 *
 * Added RFC4106 AES-GCM support for 128-bit keys under the AEAD
 * interface for 64-bit kernels.
 *    Authors: Adrian Hoban <adrian.hoban@intel.com>
 *             Gabriele Paoloni <gabriele.paoloni@intel.com>
 *             Tadeusz Struk (tadeusz.struk@intel.com)
 *             Aidan O'Mahony (aidan.o.mahony@intel.com)
 *    Copyright (c) 2010, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "crypto.h"
#include "aes.h"
#include "linux-linkage.h"
#include <stdlib.h>

#define AESNI_ALIGN	(16)
#define AES_BLOCK_MASK	(~(AES_BLOCK_SIZE-1))

asmlinkage int aesni_set_key(struct aes_cipher *ctx, const u8 *in_key,
			     unsigned int key_len);
asmlinkage void aesni_enc(struct aes_cipher *ctx, u8 *out,
			  const u8 *in);
asmlinkage void aesni_dyec(struct aes_cipher *ctx, u8 *out,
			  const u8 *in);

int crypto_fpu_init(void);
void crypto_fpu_exit(void);

static int aes_set_key_common(const struct aes_cipher *ctx,
			      const u8 *in_key, unsigned int key_len)
{
  return aesni_set_key(ctx, in_key, key_len);
}

int aes_set_key(const struct aes_cipher *ctx, const u8 *in_key,
		unsigned int key_len)
{
  return aes_set_key_common(ctx, in_key, key_len);
}

void aes_encrypt(const struct aes_cipher *ctx, u8 *dst, const u8 *src)
{
  aesni_enc(ctx, dst, src);
}

void aes_decrypt(const struct aes_cipher *ctx, u8 *dst, const u8 *src)
{
  aesni_dec(ctx, dst, src);
}

struct aes_cipher *aes_init(void)
{
  struct aes_cipher *result = malloc (sizeof (struct aes_cipher));
  if (result == NULL)
    abort();
  return result;
}
