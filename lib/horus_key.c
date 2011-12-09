
#include <horus.h>

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

void
key_to_value (char *key, int *key_len, char *string)
{
  char *p;
  char c[2];
  unsigned char val[2];
  int index = 0;
  int len = 0;

  for (p = string; *p != '\0'; p++)
    {
      c[index] = *p;
      if ('0' <= c[index] && c[index] <= '9')
        val[index] = c[index] - '0';
      else if ('a' <= c[index] && c[index] <='f')
        val[index] = c[index] - 'a' + 10;
      else if ('A' <= c[index] && c[index] <='F')
        val[index] = c[index] - 'A' + 10;
      else
        val[index] = 0;

      if (index == 1)
        key[len++] = (unsigned char) ((val[0] << 4) + val[1]);

      index ^= 1;
    }

  if (index == 1)
    {
      syslog (LOG_WARNING, "invalid key length: %d: %s",
              (int) strlen (string), string);
      key[len++] = (unsigned char) (val[0] << 4);
    }

  *key_len = len;
}

void
key_to_string (char *buf, int size, char *key, int key_len)
{
  int i;
  unsigned char val0, val1;
  int len;

  len = 0;
  for (i = 0; i < key_len; i++)
    {
      if (size - 1 <= len)
        break;

      val0 = ((unsigned char) key[i] & 0xf0) >> 4;
      if (val0 < 10)
        buf[len] = '0' + val0;
      else
        buf[len] = 'a' + (val0 - 10);
      len++;

      if (size - 1 <= len)
        break;

      val1 = (unsigned char) key[i] & 0x0f;
      if (val1 < 10)
        buf[len] = '0' + val1;
      else
        buf[len] = 'a' + (val1 - 10);
      len++;
    }

  buf[len] = '\0';
}

char *
print_key (char *key, int key_len)
{
  static char buf[SHA_DIGEST_LENGTH * 2 + 1];
  key_to_string (buf, sizeof (buf), key, key_len);
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

  /* Base case 1 */
  if (parent_x == x)
    {
      if (parent_y != y)
        {
          syslog (LOG_WARNING, "%s(): range mismatch: "
                  "x = %d, y = %d, parent_x = %d, parent_y = %d",
                  __func__, x, y, parent_x, parent_y);
          return -1;
        }

      /* key = int(parent), because key == parent */
      key_to_value (key, key_len, parent);

      return 0;
    }

  /* Base case 2 */
  if (parent_x == x - 1)
    {
      if (parent_y != y / 4)
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
  key_to_string (next_parent, sizeof (next_parent), ibuf, ibuf_len);
  next_parent_len = strlen (next_parent);

  block_key (key, key_len, next_parent, next_parent_len, x, y);

  return 0;
}

int
horus_key_by_master (char *key, int *key_len, int x, int y,
                      char *master, int master_len)
{
  char key00[SHA_DIGEST_LENGTH * 2 + 1];
  char ibuf[SHA_DIGEST_LENGTH];
  int key00_len, ibuf_len;

  if (! log_on)
    log_init ();

  if (debug)
    syslog (LOG_INFO, "%s(): x = %d, y = %d", __func__, x, y);

  block_key (ibuf, &ibuf_len, master, master_len, 0, 0);
  key_to_string (key00, sizeof (key00), ibuf, ibuf_len);
  key00_len = strlen (key00);

  horus_key_from_to (key, key_len, x, y, key00, key00_len, 0, 0);
  return 0;
}

void
horus_key (char *key, int *key_len, int filepos,
           char *ktype, char *kvalue)
{
  int x, y;
  int rkey_x, rkey_y;
  char keybuf[SHA_DIGEST_LENGTH * 2 + 1];

  x = 9;
  y = filepos / MIN_CHUNK_SIZE;

  if (! ktype || ! kvalue)
    {
      *key_len = 0;
      return;
    }

  if (! strcasecmp (ktype, "master"))
    {
      syslog (LOG_INFO, "key type: master");
      syslog (LOG_INFO, "key value: %s", kvalue);
      horus_key_by_master (key, key_len, x, y, kvalue, strlen (kvalue));
      key_to_string (keybuf, sizeof (keybuf), key, *key_len);
      syslog (LOG_INFO, "K(%d,%d) = %s", x, y, keybuf);
    }
  else
    {
      sscanf (ktype, "%d,%d", &rkey_x, &rkey_y);
      syslog (LOG_INFO, "key type: range: %d,%d", rkey_x, rkey_y);
      syslog (LOG_INFO, "key value: %s", kvalue);
      horus_key_from_to (key, key_len, x, y,
                         kvalue, strlen (kvalue), rkey_x, rkey_y);
      key_to_string (keybuf, sizeof (keybuf), key, *key_len);
      syslog (LOG_INFO, "K(%d,%d) = %s", x, y, keybuf);
    }
}

void
horus_get_key (char **ktype, char **kvalue)
{
  *ktype = getenv ("HORUS_KEY_TYPE");
  *kvalue = getenv ("HORUS_KEY_VALUE");
}


