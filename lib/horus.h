
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_OPENSSL
#include <openssl/sha.h>
#include <openssl/hmac.h>
#endif /*HAVE_OPENSSL*/

#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include <CommonCrypto/CommonHMAC.h>
#endif
#endif /*__APPLE__*/

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif /*MIN*/

extern int debug;
extern unsigned int block_size[];

int snprint_hex (char *buf, int bufsiz, char *key, int size);
void print_hex (char *prefix, char *key, int size);
char *print_key (char *key, int key_len);

char *
block_key (char *key, int *key_len,
           char *parent, int parent_len, int x, int y);

