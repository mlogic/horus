
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

int
horus_key_from_to (char *key, int *key_len, int x, int y,
                   char *parent, int parent_len, int parent_x, int parent_y)
{
  char next_parent[SHA_DIGEST_LENGTH * 2 + 1];
  char ibuf[SHA_DIGEST_LENGTH];
  int next_parent_len, ibuf_len;

  if (! log_on)
    log_init ();

  if (debug)
    syslog (LOG_INFO, "%s(): x = %d, y = %d, parent_x = %d, parent_y = %d",
            __func__, x, y, parent_x, parent_y);

  /* Base case */
  if (parent_x == x)
    {
      if (parent_y != y)
        {
          syslog (LOG_WARNING, "%s(): range mismatch: "
                  "x = %d, y = %d, parent_x = %d, parent_y = %d",
                  __func__, x, y, parent_x, parent_y);
          return -1;
        }

      block_key (key, key_len, parent, parent_len, x, y);
      return 0;
    }

  /* Recursive */
  horus_key_from_to (ibuf, &ibuf_len, x - 1, y / 4,
                     parent, parent_len, parent_x, parent_y);

  /* parent = str(key) */
  snprintf (next_parent, sizeof (next_parent), "%s",
            print_key (ibuf, ibuf_len));
  next_parent_len = strlen (next_parent);
  syslog (LOG_INFO, "x: %d y: %d next_prarent_len: %d",
          x, y, next_parent_len);

  block_key (key, key_len, next_parent, next_parent_len, x, y);

  return 0;
}

int
horus_key_by_master (char *key, int *key_len, int x, int y,
                      char *master, int master_len)
{
  if (! log_on)
    log_init ();

  if (debug)
    syslog (LOG_INFO, "%s(): x = %d, y = %d", __func__, x, y);

  horus_key_from_to (key, key_len, x, y, master, master_len, 0, 0);
  return 0;
}

void
horus_decrypt (int fd, void *buf, size_t nbyte)
{
  off_t offset;
  offset = lseek (fd, 0, SEEK_CUR);
}

void
horus_encrypt (int fd, void *buf, size_t nbyte)
{
  off_t offset;
  offset = lseek (fd, 0, SEEK_CUR);
}



