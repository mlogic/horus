/*
 * Horus AES-XTS implementation. Based on Linux kernel 3.2.3.
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Yan Li <yanli@ucsc.edu>
 *
 * XTS: as defined in IEEE1619/D16
 *      http://grouper.ieee.org/groups/1619/email/pdf00086.pdf
 *      (sector sizes which are not a multiple of 16 bytes are,
 *      however currently unsupported)
 *
 * Copyright (c) 2007 Rik Snel <rsnel@cube.dyndns.org>
 *
 * Based om ecb.c
 * Copyright (c) 2006 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <errno.h>
#include "aes.h"
#include "xts.h"
#include "b128ops.h"
#include "gf128mul.h"

int aes_xts_setkey(struct aes_xts_cipher *ctx, const u8 *key,
		   unsigned int keylen)
{
  int err;

  /* key consists of keys of equal size concatenated, therefore
   * the length must be even */
  if (keylen % 2) {
    /* tell the user why there was an error */
    return -EINVAL;
  }

  /* we need two cipher instances: one to compute the initial 'tweak'
   * by encrypting the IV (usually the 'plain' iv) and the other
   * one to encrypt and decrypt the data */

  /* tweak cipher, uses Key2 i.e. the second half of *key */
  err = aes_set_key(ctx->tweak, key + keylen/2, keylen/2);
  if (err)
    return err;

  /* data cipher, uses Key1 i.e. the first half of *key */
  err = aes_set_key(ctx->child, key, keylen/2);
  if (err)
    return err;

  return 0;
}

struct sinfo {
  be128 *t;
  struct aes_cipher *tfm;
  void (*fn)(const struct aes_cipher *, u8 *, const u8 *);
};

static inline void xts_round(struct sinfo *s, void *dst, const void *src)
{
  be128_xor(dst, s->t, src);		/* PP <- T xor P */
  s->fn(s->tfm, dst, dst);		/* CC <- E(Key1,PP) */
  be128_xor(dst, dst, s->t);		/* C <- T xor CC */
}

void aes_crypt(struct aes_xts_cipher *ctx,
	       void *dst, const void *src, size_t nbytes, void *iv,
	       void (*fn)(const struct aes_cipher *, u8 *, const u8 *))
{
  const int bs = CIPHER_BLOCK_SIZE;
  struct sinfo s = {
    .tfm = ctx->child,
    .fn = fn
  };
  u8 *wsrc = (u8 *)src;
  u8 *wdst = (u8 *)dst;

  s.t = (be128 *)iv;

  /* calculate first value of T */
  aes_encrypt(ctx->tweak, iv, iv);

  goto first;

  do {
    gf128mul_x_ble(s.t, s.t);

  first:
    xts_round(&s, wdst, wsrc);

    wsrc += bs;
    wdst += bs;
  } while ((nbytes -= bs) >= bs);
}

void aes_xts_encrypt(struct aes_xts_cipher *ctx, void *dst,
	    const void *src, size_t nbytes, void *iv)
{
  aes_crypt(ctx, dst, src, nbytes, iv,
	aes_encrypt);
}

void aes_xts_decrypt(struct aes_xts_cipher *ctx, void *dst,
		     const void *src, size_t nbytes, void *iv)
{
  aes_crypt(ctx, dst, src, nbytes, iv,
	aes_decrypt);
}

struct aes_xts_cipher* aes_xts_init()
{
  struct aes_xts_cipher *result = malloc (sizeof (struct aes_xts_cipher));
  if (result == NULL)
    abort();

  result->child = aes_init ();
  result->tweak = aes_init ();

  return result;
}
