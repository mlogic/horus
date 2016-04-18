/*
 * Horus AES Test
 *
 * Developers:
 *  Yan Li <yanli@ascar.io>
 *
 * Copyright (c) 2012, The Regents of the University of California
 * All rights reserved.
 * Created by the Storage Systems Research Center, http://www.ssrc.ucsc.edu
 * Department of Computer Science
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Storage Systems Research Center, the
 *       University of California, nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xts.h"

int debug = 0;

/** Test encrypt/decrypt
 * Encrypt a string, then decrypt it, compare with plaintext.
 */
int main(int argc, char **argv)
{
  const int test_buf_size=32;
  const int keylen=16*2; // xts requires two keys
  u8 key[keylen];
  u8 iv[keylen];
  u8 plain_buf[test_buf_size];
  u8 enc_buf[test_buf_size];
  u8 dec_buf[test_buf_size];
  int i;

  if (argc == 2 && strcmp ("-d", argv[1]) == 0)
    debug = 1;

  for (i=0; i<keylen; ++i)
    {
      key[i] = i;
      iv[i] = i;
    }

  /* write some text into plaintext buffer */
  for (i=0; i<test_buf_size; ++i)
    plain_buf[i] = '0'+ i % 10;

  /* zero enc_buf before using it, so we can know if the data is
     really stored into enc_buf later during debugging */
  memset (enc_buf, 0, sizeof(enc_buf));

  struct aes_xts_cipher *c = aes_xts_init();
  if (aes_xts_setkey (c, key, keylen) != 0)
    {
      printf ("aes_xts_setkey error!");
      abort ();
    }

  aes_xts_encrypt (c, enc_buf, plain_buf, test_buf_size, iv);

  if (debug)
    {
      printf ("Plain text:\n");
      for (i=0; i<test_buf_size; ++i)
	{
	  printf ("%02x ", plain_buf[i]);
	}
      printf ("\nEncrypted text:\n");
      for (i=0; i<test_buf_size; ++i)
	{
	  printf ("%02x ", enc_buf[i]);
	}
    }

  /* reset IV, because aes_xts_encrypt would change IV passed to it */
  for (i=0; i<keylen; ++i)
    iv[i] = i;
  aes_xts_decrypt (c, dec_buf, enc_buf, test_buf_size, iv);

  if (debug)
    {
      printf ("\nDecrypted text:\n");
      for (i=0; i<test_buf_size; ++i)
	{
	  printf ("%02x ", dec_buf[i]);
	}
    }

  for (i=0; i<test_buf_size; ++i)
    {
      if (plain_buf[i] != dec_buf[i])
	{
	  printf ("AES test failed\n");
	  abort();
	}
    }

  return 0;
}
