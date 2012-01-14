
#include <horus.h>

#include <horus_key.h>
#include <horus_crypt.h>

void
horus_coding_xor (char *buf, int size, char *key, int key_len)
{
  int i;
  char ch;
  for (i = 0; i < size; i++)
    {
      ch = buf[i];
      buf[i] = ch ^ (unsigned char) key[i % key_len];
    }
}

void
horus_coding (int fd, int fdpos, char *buf, size_t nbyte,
              char *ktype, char *kvalue)
{
  char buffer[MIN_CHUNK_SIZE];
  size_t rest = nbyte;
  off_t bufp;
  off_t offset;
  int ncopy;
  char key[SHA_DIGEST_LENGTH];
  int key_len;

  rest = nbyte;
  bufp = 0;

  /* calculate process length of this chunk */
  offset = fdpos % MIN_CHUNK_SIZE;
  ncopy = MIN (MIN_CHUNK_SIZE - offset, rest);

  /* copy to temporary buffer */
  memcpy (&buffer[offset], &buf[bufp], ncopy);

  /* get key */
  horus_key (key, &key_len, fdpos, ktype, kvalue);
  if (key_len == 0)
    {
      syslog (LOG_WARNING, "failed to get keys. quit coding.");
      return;
    }

  /* encode/decode (xor) */
  horus_coding_xor (buffer, sizeof (buffer), key, key_len);

  /* write back */
  memcpy (&buf[bufp], &buffer[offset], ncopy);

  bufp += ncopy;
  fdpos += ncopy;
  rest -= ncopy;

  while (rest > 0)
    {
      /* calculate process length of this chunk */
      ncopy = MIN (MIN_CHUNK_SIZE, rest);

      /* copy to temporary buffer */
      memcpy (&buffer[0], &buf[bufp], ncopy);

      /* get key */
      horus_key (key, &key_len, fdpos, ktype, kvalue);
      if (key_len == 0)
        {
          syslog (LOG_WARNING, "failed to get keys. quit coding.");
          return;
        }

      /* encode/decode (xor) */
      horus_coding_xor (buffer, sizeof (buffer), key, key_len);

      /* write back */
      memcpy (&buf[bufp], &buffer[0], ncopy);

      bufp += ncopy;
      fdpos += ncopy;
      rest -= ncopy;
    }
}

void
horus_decrypt (int fd, int fdpos, char *buf, size_t nbyte,
               char *ktype, char *kvalue)
{
  horus_coding (fd, fdpos, buf, nbyte, ktype, kvalue);
}

void
horus_encrypt (int fd, int fdpos, char *buf, size_t nbyte,
               char *ktype, char *kvalue)
{
  horus_coding (fd, fdpos, buf, nbyte, ktype, kvalue);
}


