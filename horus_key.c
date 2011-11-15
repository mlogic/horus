
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HAVE_OPENSSL /* XXX */
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

char *progname;

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096

unsigned int block_size[] = {
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 9,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 8,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 7,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 6,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 5,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 4,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 3,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 2,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 1,
  MIN_CHUNK_SIZE << BRANCH_FACTOR_BITS * 0,
};

char *
block_key (char *key, int *key_len,
           char *parent, int parent_len, int x, int y)
{
  u_int32_t x__y = ((x << 24) | y);
  void *message = &x__y;
  int size = sizeof (x__y);

#ifdef __APPLE__
  CCHmac (kCCHmacAlgSHA1, parent, parent_len,
          (const void *)message, size, key);
  *key_len = SHA_DIGEST_LENGTH;
#else /*!__APPLE__*/
  HMAC (EVP_sha1(), parent, parent_len,
        (const unsigned char *) message, size,
        key, key_len);
#endif /*__APPLE__*/

  return key;
}

char *
key_print (char *key, int key_len)
{
  int i;
  static char string[SHA_DIGEST_LENGTH * 2 + 1];

  for (i = 0; i < key_len; i++)
    {
      snprintf (&string[i * 2], 3, "%02x", key[i]);
    }

  return string;
}

#define MIN(x,y) ((x) < (y) ? (x) : (y))

void
usage ()
{
  printf ("%s [offset] [master_key]\n", progname);
  exit (0);
}

int
main (int argc, char **argv)
{
  int i;
  int x, y;
  int offset;
  int key_len, parent_len;

  char parent[SHA_DIGEST_LENGTH];
  char key[SHA_DIGEST_LENGTH];

  progname = argv[0];

  /* Help */
  if (argc > 1)
    {
      if (! strcmp (argv[1], "-h"))
        usage ();
      if (! strcmp (argv[1], "--help"))
        usage ();
    }

  /* Byte offset */
  offset = 4096;
  if (argc > 1)
    {
      offset = atoi (argv[1]);
    }

  /* Master key */
  strncpy (key, "master", sizeof (key));
  key_len = strlen ("master");
  if (argc > 2)
    {
      strncpy (key, argv[2], SHA_DIGEST_LENGTH);
      key_len = MIN (SHA_DIGEST_LENGTH, strlen (argv[2]));
    }

  printf ("offset = %010d, master = %s(%d).\n",
          offset, key_print (key, key_len), key_len);

  /* Out of bound offset. */
  if (offset >= block_size[0])
    {
      printf ("offset out of bound: %d >= %d\n", offset, block_size[0]);
      exit (1);
    }

#if 0
  for (i = 0; i < 10; i++)
    printf ("block_size[%d] = %10d\n", i, block_size[i]);
#endif

  for (i = 0; i < 10; i++)
    {
      x = i;
      y = offset / block_size[x];

      /* parent = key. */
      strncpy (parent, key, sizeof (parent));
      parent_len = key_len;

      if (x != 0)
        {
          /* calculate K_(x,y). */
          block_key (key, &key_len, parent, parent_len, x, y);
        }

      printf ("K_(%d,%d) = %s(%d) [%010d-%010d]\n",
               x, y, key_print (key, key_len), key_len,
               block_size[x] * y, block_size[x] * (y + 1) -1);
    }

  return 0;
}



