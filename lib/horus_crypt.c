#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <log.h>

#include <aes.h>
#include <xts.h>

char *kds_serv_addr = "128.114.52.64";
char *file_path = "/scratch/horus/horus-data-file";
int request_level = -1;

/* Horus configuration */
int leaf_level = -1;
int horus_sockfd = -1;
struct horus_file_config horus_config;
struct sockaddr_in horus_kds_addr;

/* Horus key cache */
char horus_key[HORUS_KEY_LEN];
size_t horus_key_len = -1;
int horus_key_x = -1;
int horus_key_y = -1;

/* AES cipher */
struct aes_xts_cipher *cipher = NULL;
unsigned long aes_block_size = 0;

#define OP_DECRYPT 0
#define OP_ENCRYPT 1

void
horus_crypt (char *buf, ssize_t size, unsigned long long offset, int op)
{
  unsigned long long prev_horus_align, next_horus_align;
  unsigned long long ioffset = 0;
  unsigned long sblock, eblock, block_id;
  int ret, verbose = 0, debug = 0;
  unsigned long x, y;
  int horus = 0, aescrypt = 0, nowriteback = 0, aggregate = 0, spinwait = 0;
  char *addr, *filename, *hostname;

  char block_key[HORUS_KEY_LEN];
  size_t block_key_len;

  /* AES */
  unsigned long aes_sblock, aes_eblock, aes_block_id;
  u_int8_t aeskey[AES_KEYSIZE_128 * 2];
  u_int8_t iv[AES_KEYSIZE_128 * 2];
  char crypt_buf[HORUS_BLOCK_SIZE];

  if (getenv ("HORUS_VERBOSE"))
    verbose++;
  if (getenv ("HORUS_DEBUG"))
    debug++;
  if (getenv ("DISABLE_WRITEBACK"))
    nowriteback++;

  if (getenv ("ENABLE_AES"))
    aescrypt++;
  if (getenv ("ENABLE_HORUS"))
    horus++;
  if (getenv ("ENABLE_AGGREGATE"))
    aggregate++;
  if (getenv ("ENABLE_SPINWAIT"))
    spinwait++;

  hostname = getenv ("HOSTNAME");

  addr = getenv ("HORUS_KDS_SERVER");
  if (addr)
    kds_serv_addr = addr;
  filename = getenv ("HORUS_FILENAME");
  if (filename)
    file_path = filename;

  printf ("Horus BTIO: %s%s%s%s%s%s%s%s%s%s%s\n",
          (verbose ?     " verbose" : ""),
          (debug ?       " debug" : ""),
          (nowriteback ? " nowriteback" : ""),
          (aescrypt ?    " aes" : ""),
          (horus ?       " horus" : ""),
          (aggregate ?   " aggregate" : ""),
          (spinwait ?    " spinwait" : ""),
          (addr ?        " serv: " : ""),
          (addr ?          addr : ""),
          (filename ?    " file: " : ""),
          (filename ?      filename : ""));

  if (aggregate)
    request_level = 0;

  if (debug)
    horus_verbose++;

  /* Decide AES block size (only the first time) */
  if (aes_block_size == 0)
    {
      if (size > HORUS_BLOCK_SIZE)
        {
          if (size % HORUS_BLOCK_SIZE)
            aes_block_size = size % HORUS_BLOCK_SIZE;
          else
            aes_block_size = HORUS_BLOCK_SIZE;
        }
      else
        aes_block_size = size;

      if (verbose)
        printf ("AES block size: %lu\n", aes_block_size);

      if (aes_block_size % AES_KEYSIZE_128 != 0)
        printf ("aes_block_size not aligned with %d\n", AES_KEYSIZE_128);
      if (offset % aes_block_size != 0)
        printf ("offset not aligned with %lu\n", aes_block_size);
      if (size % aes_block_size != 0)
        printf ("size not aligned with %lu\n", aes_block_size);
    }

  /* Horus file config (only the first time) */
  if (horus && leaf_level < 0)
    {
      int fd;

      if (verbose)
        printf ("Horus for %s\n", file_path);

      /* Get Horus config */
      fd = open (file_path, O_RDONLY);
      ret = horus_get_file_config (fd, &horus_config);
      assert (ret >= 0 && horus_is_valid_config (&horus_config));
      close (fd);
      leaf_level = horus_get_leaf_level (&horus_config);

      /* Setup connection with KDS */
      horus_sockfd = socket (PF_INET, SOCK_DGRAM, 0);
      if (spinwait)
        fcntl (horus_sockfd, F_SETFL, O_NONBLOCK);
      assert (horus_sockfd > 0);
      memset (&horus_kds_addr, 0, sizeof (horus_kds_addr));
      horus_kds_addr.sin_family = AF_INET;
      inet_pton (AF_INET, kds_serv_addr, &horus_kds_addr.sin_addr);
      horus_kds_addr.sin_port = htons (HORUS_KDS_SERVER_PORT);
    }

  /* AES */
  if (aescrypt && cipher == NULL)
    cipher = aes_xts_init ();
  memset (aeskey, 0, sizeof (aeskey));
  memset (iv, 0, sizeof (iv));

  sblock = offset / HORUS_BLOCK_SIZE;
  eblock = (offset + size) / HORUS_BLOCK_SIZE + 1;

  x = leaf_level;
  for (block_id = sblock; block_id < eblock; block_id++)
    {
      y = block_id;

      if (horus)
        {
          /* If the horus key does not match, request */
          if (horus_key_x < 0 ||
              horus_key_y != horus_key_y_of (horus_key_x, x, y,
                                             horus_config.kht_block_size))
            {
              if (request_level < 0)
                request_level = leaf_level;
              horus_key_x = request_level;
              horus_key_y = horus_key_y_of (request_level, x, y,
                                            horus_config.kht_block_size);
              horus_key_len = sizeof (horus_key);
              if (spinwait)
                horus_key_request_spin (horus_key, &horus_key_len,
                                        file_path, horus_key_x, horus_key_y,
                                        horus_sockfd, &horus_kds_addr);
              else
                horus_key_request (horus_key, &horus_key_len,
                                   file_path, horus_key_x, horus_key_y,
                                   horus_sockfd, &horus_kds_addr);

              if (verbose)
                printf ("request K_%d,%d = %s\n", horus_key_x, horus_key_y,
                        print_key (horus_key, horus_key_len));
            }

          /* Calculate the leaf key */
          if (horus_key_x != x)
            {
              block_key_len = sizeof (block_key);
              horus_block_key (block_key, &block_key_len, x, y,
                               horus_key, horus_key_len,
                               horus_key_x, horus_key_y,
                               horus_config.kht_block_size);
              if (verbose)
                printf ("calculated K_%lu,%lu = %s\n", x, y,
                        print_key (block_key, block_key_len));
            }
          else
            {
              assert (horus_key_y == y);
              memcpy (block_key, horus_key, HORUS_KEY_LEN);
              block_key_len = horus_key_len;
            }
        }

      if (aescrypt)
        {
          /* Set AES key */
          memset (aeskey, 0, AES_KEYSIZE_128 * 2);
          if (horus)
            memcpy (aeskey, block_key, block_key_len);
          ret = aes_xts_setkey (cipher, aeskey, AES_KEYSIZE_128 * 2);
          assert (! ret);
        }

      /* Calculate AES block */
      prev_horus_align = ((offset + ioffset) / HORUS_BLOCK_SIZE) *
                         HORUS_BLOCK_SIZE;
      next_horus_align = prev_horus_align + HORUS_BLOCK_SIZE;
      aes_sblock = (offset + ioffset) / aes_block_size;
      aes_eblock = MIN (offset + size, next_horus_align) / aes_block_size;
      if (verbose)
        printf ("aes_block: start: %lu end: %lu (pos %#llx)\n",
                aes_sblock, aes_eblock, offset + ioffset);
      if (debug && aes_sblock == aes_eblock)
        {
          printf ("prev_horus_align = %llu\n", prev_horus_align);
          printf ("next_horus_align = %llu\n", next_horus_align);
          printf ("offset: %llu, ioffset: %llu, aes_block_size: %lu",
                  offset, ioffset, aes_block_size);
          printf ("MIN(offset+size,next_horus_align): %llu",
                  MIN (offset+size, next_horus_align));
        }
      //assert (aes_sblock < aes_eblock);

      for (aes_block_id = aes_sblock; aes_block_id < aes_eblock;
           aes_block_id++)
        {
          if (verbose)
            printf ("offset: %llu aes[%lu]: ioffset: %llu buf: %s\n",
                    offset, aes_block_id, ioffset,
                    print_key (buf + ioffset, 16));

          /* Set AES IV */
          memset (iv, 0, sizeof (iv));
          *(unsigned long long *)iv = aes_block_id;

          /* AES cryptography */
          if (op == OP_DECRYPT)
            {
              if (aescrypt && debug)
                printf ("offset: %llu aes[%lu]: ioffset: %llu op: decrypt\n",
                        offset, aes_block_id, ioffset);
              if (aescrypt)
                aes_xts_decrypt (cipher, crypt_buf, buf + ioffset,
                                 aes_block_size, iv);
              else
                memcpy (crypt_buf, buf + ioffset, aes_block_size);
            }
          else
            {
              if (aescrypt && debug)
                printf ("offset: %llu aes[%lu]: ioffset: %llu op: encrypt\n",
                        offset, aes_block_id, ioffset);
              if (aescrypt)
                aes_xts_encrypt (cipher, crypt_buf, buf + ioffset,
                                 aes_block_size, iv);
              else
                memcpy (crypt_buf, buf + ioffset, aes_block_size);
            }

          if (debug)
            printf ("offset: %llu aes[%lu]: ioffset: %llu crypt: %s\n",
                    offset, aes_block_id, ioffset,
                    print_key (crypt_buf, 16));

          if (! nowriteback)
            {
              /* write it back */
              memcpy (buf + ioffset, crypt_buf, aes_block_size);
            }

          if (debug)
            printf ("offset: %llu aes[%lu]: ioffset: %llu writeback: %s\n",
                    offset, aes_block_id, ioffset,
                    print_key (buf + ioffset, 16));

          ioffset += aes_block_size;
        }
    }
}

void
horus_encrypt (char *buf, ssize_t size, unsigned long long offset)
{
  horus_crypt (buf, size, offset, OP_ENCRYPT);
}

void
horus_decrypt (char *buf, ssize_t size, unsigned long long offset)
{
  horus_crypt (buf, size, offset, OP_DECRYPT);
}



