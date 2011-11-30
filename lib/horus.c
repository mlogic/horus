
#include <horus.h>

int debug = 0;

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

