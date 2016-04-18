/*
 * Horus AES Cipher. Based on Linux kernel 3.2.3.
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ascar.io>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 */

#ifndef _AES_H
#define _AES_H

/*
 * Common values for AES algorithms
 */

#include "crypto.h"

#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32
#define AES_KEYSIZE_128		16
#define AES_KEYSIZE_192		24
#define AES_KEYSIZE_256		32
#define AES_BLOCK_SIZE		16
#define AES_MAX_KEYLENGTH	(15 * 16)
#define AES_MAX_KEYLENGTH_U32	(AES_MAX_KEYLENGTH / sizeof(u32))

/*
 * Please ensure that the first two fields are 16-byte aligned
 * relative to the start of the structure, i.e., don't move them!
 */
struct aes_cipher {
	u32 key_enc[AES_MAX_KEYLENGTH_U32];
	u32 key_dec[AES_MAX_KEYLENGTH_U32];
	u32 key_length;
};

extern const u32 crypto_ft_tab[4][256];
extern const u32 crypto_fl_tab[4][256];
extern const u32 crypto_it_tab[4][256];
extern const u32 crypto_il_tab[4][256];

int aes_expand_key(struct aes_cipher *ctx, const u8 *in_key,
		   unsigned int key_len);

struct aes_cipher *aes_init(void);
int aes_set_key(const struct aes_cipher *ctx, const u8 *key,
		unsigned int keylen);
void aes_encrypt (const struct aes_cipher *ctx, u8 *dst, const u8 *src);
void aes_decrypt (const struct aes_cipher *ctx, u8 *dst, const u8 *src);

#endif /* _AES_H */
