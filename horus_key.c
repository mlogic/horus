
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

int debug = 0;
char *progname;

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096

unsigned int block_size[] = {
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 9),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 8),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 7),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 6),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 5),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 4),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 3),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 2),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 1),
  MIN_CHUNK_SIZE << (BRANCH_FACTOR_BITS * 0),
};

#define MIN(x,y) ((x) < (y) ? (x) : (y))

int
snprint_hex (char *buf, int bufsiz, char *key, int size)
{
  int i;
  for (i = 0; i < MIN ((bufsiz - 1) / 2, size); i++)
    snprintf (&buf[i * 2], 3, "%02x", (unsigned char) key[i]);
  return i;
}

void
print_hex (char *prefix, char *key, int size)
{
  int i;
  printf ("%s", prefix);
  for (i = 0; i < size; i++)
    printf ("%02x", (unsigned char) key[i]);
  printf (" (size: %d)\n", size);
}

char *
print_key (char *key, int key_len)
{
  static char buf[SHA_DIGEST_LENGTH * 2 + 1];
  snprint_hex (buf, sizeof (buf), key, key_len);
  return buf;
}

char *
block_key (char *key, int *key_len,
           char *parent, int parent_len, int x, int y)
{
  void *message;
  int size;

#ifdef NON_STRING_MESSAGE
  u_int32_t x__y = ((x << 24) | y);
  message = &x__y;
  size = sizeof (x__y);
#else /*NON_STRING_MESSAGE*/
  char x__y[17];
  message = x__y;
  size = sizeof (x__y) - 1;
  snprintf (x__y, sizeof (x__y), "%016x", ((x << 24) | y));
#endif /*NON_STRING_MESSAGE*/

  if (debug)
    {
      print_hex ("parent = ", parent, parent_len);
      printf ("x: %d, y: %d, message: %s, size: %d\n", x, y, x__y, size);
      print_hex ("message = ", x__y, size);
    }

#ifdef __APPLE__
  CCHmac (kCCHmacAlgSHA1, parent, parent_len,
          (const void *)message, size, key);
  *key_len = SHA_DIGEST_LENGTH;
#else /*__APPLE__*/
  HMAC (EVP_sha1(), parent, parent_len,
        (const unsigned char *) message, size,
        key, key_len);
#endif /*__APPLE__*/

  if (debug)
    print_hex ("key = ", key, *key_len);

  return key;
}

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
  int offset = 4096;
  char *master = "master";

  char parent[SHA_DIGEST_LENGTH * 2 + 1];
  char key[SHA_DIGEST_LENGTH];
  int parent_len, key_len;

  progname = argv[0];

  /* Help */
  if (argc > 1)
    {
      if (! strcmp (argv[1], "-h"))
        usage ();
      if (! strcmp (argv[1], "--help"))
        usage ();
      if (! strcmp (argv[1], "-d"))
        debug++;
      if (! strcmp (argv[1], "--debug"))
        debug++;
    }

  /* Byte offset */
  if (argc > 1)
    offset = atoi (argv[1]);

  /* Master key */
  if (argc > 2)
    master = argv[2];
  snprintf (parent, sizeof (parent), "%s", master);
  parent_len = strlen (master);

  printf ("offset = %010d, master = %s(%d).\n",
          offset, print_key (parent, parent_len), parent_len);

  /* Out of bound offset. */
  if (offset >= block_size[0])
    {
      printf ("offset out of bound: %d >= %d\n", offset, block_size[0]);
      exit (1);
    }

#ifdef DEBUG
  for (i = 0; i < 10; i++)
    printf ("block_size[%d] = %10d\n", i, block_size[i]);
#endif /*DEBUG*/

  for (i = 0; i < 10; i++)
    {
      x = i;
      y = offset / block_size[x];

#ifdef NON_SHA1_ROOT_KEY
      if (x != 0)
        {
#endif /*NON_SHA1_ROOT_KEY*/

          /* calculate K_(x,y). */
          block_key (key, &key_len, parent, parent_len, x, y);

#ifdef NON_SHA1_ROOT_KEY
        }
#endif /*NON_SHA1_ROOT_KEY*/

      printf ("K_(%d,%d) = %s(%d) [%010d-%010d]\n",
               x, y, print_key (key, key_len), key_len,
               block_size[x] * y, block_size[x] * (y + 1) -1);

      /* parent = str(key) */
      snprintf (parent, sizeof (parent), "%s", print_key (key, key_len));
      parent_len = strlen (parent); // will be 40.
    }

  return 0;
}



