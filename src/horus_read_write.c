#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <horus_attr.h>
#include <horus_stats.h>
#include <benchmark.h>

#include "timeval.h"
#include "minmax.h"

#include <aes.h>
#include <xts.h>

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;

#define HORUS_MAX_PREFETCH_KEY 9216

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern int horus_debug;
extern int horus_verbose;

int benchmark = 0;
int aggregate = 0;

const char *optstring = "hvdbegus:a:p:rwf:i:o:l:t:n:";
const char *optusage = "\
-h, --help        Display this help and exit\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-b, --benchmark   Turn on benchmarking\n\
-e, --encrypt     Turn on encrypting\n\
-g, --decrypt     Turn on decrypting\n\
-u, --horus       Turn on Horus mode\n\
-s, --server      Specify server IP address A.B.C.D[:P] (default: %s:%d)\n\
-a, --aggregate   Specify the level of aggregation of request range keys\n\
-p, --prefetch    Specify the number of key to prefetch\n\
-r, --read        Do read\n\
-w, --write       Do write\n\
-f, --file        Specify the file name\n\
-i, --input-file  Specify the input file name\n\
-o, --output-file Specify the output file name\n\
-l, --length      Specify size of the file (in bytes, e.g., 1G)\n\
-t, --size        Specify size of the read/write unit (in bytes, e.g., 1G)\n\
-n, --ncount      Specify the number of times to read/write\n\
";

const struct option longopts[] = {
  { "help",       no_argument,        NULL, 'h' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "benchmark",  no_argument,        NULL, 'b' },
  { "encrypt",    no_argument,        NULL, 'e' },
  { "decrypt",    no_argument,        NULL, 'g' },
  { "horus",      no_argument,        NULL, 'u' },
  { "server",     required_argument,  NULL, 's' },
  { "aggregate",  required_argument,  NULL, 'a' },
  { "prefetch",   required_argument,  NULL, 'p' },
  { "read",       no_argument,        NULL, 'r' },
  { "write",      no_argument,        NULL, 'w' },
  { "file",       required_argument,  NULL, 'f' },
  { "input-file", required_argument,  NULL, 'i' },
  { "output-file",required_argument,  NULL, 'o' },
  { "length",     required_argument,  NULL, 'l' },
  { "size",       required_argument,  NULL, 't' },
  { "ncount",     required_argument,  NULL, 'n' },
  { NULL,         0,                  NULL,  0  }
};

void
usage ()
{
  printf ("Usage : %s [OPTION...] <filename>\n", progname);
  printf ("\n");
  printf ("options are \n");
  printf (optusage, HORUS_KDS_SERVER_ADDR, HORUS_KDS_SERVER_PORT);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

void
print_block (char *block_data)
{
  unsigned long i;
  for (i = 0; i < HORUS_BLOCK_SIZE; i++)
    {
      if (i % 32 == 0)
        printf ("data: %4lu: ", i);
      printf ("%02x", (char) block_data[i] & 0xff);
      if (i % 32 == 31)
        printf ("\n");
    }
}

void
horus_key_prefetch (char *filename, int x, int y, int num,
                    int sockfd, struct sockaddr_in *serv,
                    struct key_response_packet *prefetch_key)
{
  int ret;
  struct key_request_packet req;
  struct key_response_packet res;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  int resx, resy, reskeylen;
  int reserr, ressuberr;
  int i, received;

  for (i = 0; i < num; i++)
    {
      memset (&req, 0, sizeof (req));
      req.x = htonl (x);
      req.y = htonl (y + i);
      snprintf (req.filename, sizeof (req.filename), "%s", filename);

      ret = sendto (sockfd, &req, sizeof (struct key_request_packet), 0,
                    (struct sockaddr *) serv, sizeof (struct sockaddr_in));
      assert (ret == sizeof (struct key_request_packet));

      if (horus_verbose)
        printf ("prefetch requested key: K_%d,%d\n", x, y + i);
    }

  received = 0;
  while (received < num)
    {
      ret = recvfrom (sockfd, &res, sizeof (struct key_response_packet),  0,
                      (struct sockaddr *) &addr, &addrlen);
      assert (ret == sizeof (struct key_response_packet));

      resx = ntohl (res.x);
      resy = ntohl (res.y);
      if (x != resx || resy < y || y + num < resy)
        {
          if (horus_verbose)
            printf ("wrong key: K_%d,%d\n", resx, resy);
          continue;
        }

      if (horus_verbose)
        printf ("prefetch received key[%d]: K_%d,%d\n", received, resx, resy);

      memcpy (&prefetch_key[resy - y], &res,
              sizeof (struct key_response_packet));
      received++;
    }
}

int
main (int argc, char **argv)
{
  int fd = -1, inputfd = -1, outputfd = -1;
  int sockfd = -1;
  int ret = 0, ch;
  unsigned long i, j;
  unsigned long startb, endb;
  int ncount = 0;
  unsigned long long offset, length, size;
  char *filename = NULL, *inputfile = NULL, *outputfile = NULL;
  char *endptr;
  int encrypt, decrypt, horus, readflag, writeflag;
  int prefetch = 0;
  struct key_response_packet *prefetch_key = NULL;
  unsigned long leaf_level = HORUS_DEFAULT_LEAF_LEVEL;
  unsigned long level, boffset, nblock;
  unsigned long alevel, aboffset, anblock;
  unsigned long rlevel, rboffset, rnblock;
  int branch;

  u_int16_t port;
  char *portspec = NULL;
  char *server[HORUS_MAX_SERVER_NUM];
  struct sockaddr_in serv_addr[HORUS_MAX_SERVER_NUM];
  int nservers = 0;

  int oflag;
  struct horus_file_config c;

  char *block;
  char block_data[HORUS_BLOCK_SIZE];
  char block_storage[HORUS_BLOCK_SIZE];

  char key[HORUS_KEY_LEN];
  size_t key_len = HORUS_KEY_LEN;
  char rkey[HORUS_KEY_LEN];
  size_t rkey_len = HORUS_KEY_LEN;

  u_int8_t aeskey[AES_KEYSIZE_128 * 2];
  u_int8_t iv[AES_KEYSIZE_128 * 2];
  struct aes_xts_cipher *cipher;

  unsigned long total, total_read, total_write;
  unsigned long total_encrypt, total_decrypt;
  struct timeval start, end, res;
  struct timeval startnet, endnet, resnet, sumnet;

  memset (&serv_addr, 0, sizeof (serv_addr));
  horus = encrypt = decrypt = aggregate = 0;
  readflag = writeflag = 0;
  alevel = anblock = aboffset = 0;
  rlevel = rnblock = rboffset = 0;
  offset = length = size = 0;

  for (i = 0; i < HORUS_BLOCK_SIZE; i++)
    block_data[i] = (char) i;
  memset (key, 0, sizeof (key));
  memset (rkey, 0, sizeof (rkey));
  memset (iv, 0, sizeof (iv));

  total = total_read = total_write = 0;
  total_encrypt = total_decrypt = 0;

  progname = (1 ? "kds_client" : argv[0]);
  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'v':
          horus_verbose++;
          break;
        case 'd':
          horus_debug++;
          break;
        case 'b':
          benchmark++;
          break;
        case 'e':
          encrypt++;
          break;
        case 'g':
          decrypt++;
          break;
        case 'u':
          horus++;
          break;
        case 's':
          if (nservers >= HORUS_MAX_SERVER_NUM)
            break;
          server[nservers] = optarg;
          portspec = index (optarg, ':');
          if (portspec)
            *portspec++ = '\0';
          serv_addr[nservers].sin_family = AF_INET;
          ret = inet_pton (AF_INET, server[nservers],
                           &serv_addr[nservers].sin_addr);
          if (ret != 1)
            {
              fprintf (stderr, "invalid server address: %s\n", optarg);
              return -1;
            }
          if (portspec)
            {
              port = (u_int16_t) strtol (portspec, &endptr, 0);
              if (*endptr != '\0')
                {
                  fprintf (stderr, "invalid port number \'%c\' in %s\n",
                           *endptr, portspec);
                  return -1;
                }
              serv_addr[nservers].sin_port = htons (port);
            }
          else
            serv_addr[nservers].sin_port = htons (HORUS_KDS_SERVER_PORT);
          nservers++;
          break;
        case 'a':
          aggregate++;
          alevel = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          if (alevel < 0 || HORUS_MAX_KHT_DEPTH <= alevel)
            {
              fprintf (stderr, "invalid alevel: %lu\n", alevel);
              return -1;
            }
          break;
        case 'p':
          prefetch = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          if (prefetch < 0 || HORUS_MAX_PREFETCH_KEY < prefetch)
            {
              fprintf (stderr, "invalid prefetch: %d\n", prefetch);
              return -1;
            }
          break;
        case 'r':
          readflag++;
          break;
        case 'w':
          writeflag++;
          break;
        case 'f':
          filename = optarg;
          break;
        case 'i':
          inputfile = optarg;
          break;
        case 'o':
          outputfile = optarg;
          break;
        case 'l':
          length = canonical_byte_size (optarg, &endptr);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 't':
          size = canonical_byte_size (optarg, &endptr);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          if (size < HORUS_BLOCK_SIZE)
            {
              fprintf (stderr, "invalid read/write block size: %llu\n", size);
              return -1;
            }
          break;
        case 'n':
          ncount = (int) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        default:
          usage ();
          break;
        }
    }
  argc -= optind;
  argv += optind;

  if (argc > 0)
    {
      filename = argv[0];
      argc--;
      argv++;
    }

  if (filename == NULL)
    {
      printf ("specify target filename.\n");
      exit (1);
    }

  if (argc > 0)
    {
      printf ("unknown arguments ignored:");
      for (i = 0; i < argc; i++)
        printf (" %s", argv[i]);
      printf ("\n");
    }

  if (horus && nservers == 0)
    {
      /* default setting */
      server[nservers] = HORUS_KDS_SERVER_ADDR;
      serv_addr[nservers].sin_family = AF_INET;
      ret = inet_pton (AF_INET, server[nservers],
                       &serv_addr[nservers].sin_addr);
      assert (ret == 1);
      port = HORUS_KDS_SERVER_PORT;
      serv_addr[nservers].sin_port = htons (port);
      nservers++;
    }

  oflag = O_RDONLY;
  if (readflag && writeflag)
    oflag = O_RDWR;
  else if (writeflag)
    oflag = O_WRONLY;

  fd = open (filename, oflag);
  if (fd < 0)
    {
      printf ("cannot open file: %s\n", filename);
      exit (1);
    }

  if (inputfile)
    {
      inputfd = open (inputfile, O_RDONLY);
      if (inputfd < 0)
        {
          printf ("cannot open input file: %s\n", inputfile);
          exit (1);
        }
    }
  if (outputfile)
    {
      outputfd = open (outputfile, O_WRONLY | O_CREAT, 0644);
      if (outputfd < 0)
        {
          printf ("cannot open output file: %s\n", outputfile);
          exit (1);
        }
    }

  memset (&c, 0, sizeof (c));
  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      printf ("cannot read Horus config: %s\n", filename);
      exit (1);
    }

  if (! horus_is_valid_config (&c))
    {
      printf ("invalid Horus config: %s\n", filename);
      exit (1);
    }

  /* calculate leaf level */
  for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
    if (c.kht_block_size[i])
      leaf_level = i;
  if (horus_verbose)
    printf ("leaf level: %lu\n", leaf_level);
  if (aggregate)
    {
      if (alevel > leaf_level)
        {
          alevel = leaf_level;
          printf ("aggregate level adjusted to leaf: %lu\n", alevel);
        }
    }

  /* length */
  level = leaf_level;
  boffset = 0;
  nblock = 512 * 1024;
  if (length)
    nblock = length / HORUS_BLOCK_SIZE;

  /* aggregate */
  if (aggregate)
    {
      aboffset = boffset;
      anblock = nblock;
      /* calculate range as the aggregate level */
      for (i = level; alevel < i; i--)
        {
          branch = c.kht_block_size[i - 1] / c.kht_block_size[i];
          anblock = (anblock / branch) + (anblock % branch ? 1 : 0);
          aboffset /= branch;
        }
    }

  if (horus_verbose || benchmark)
    {
      printf ("filename: %s\n", filename);
      if (inputfile)
        printf ("input file: %s\n", inputfile);
      if (outputfile)
        printf ("output file: %s\n", outputfile);
      if (benchmark)
        printf ("benchmark.\n");
      printf ("level: %lu boffset: %lu nblock: %lu\n",
              level, boffset, nblock);
      if (aggregate)
        printf ("alevel: %lu aboffset: %lu anblock: %lu\n",
                alevel, aboffset, anblock);
      if (prefetch)
        printf ("prefetch: %d\n", prefetch);
      printf ("flags: ");
      if (encrypt)
        printf (" encrypt");
      if (decrypt)
        printf (" decrypt");
      if (readflag)
        printf (" readflag");
      if (writeflag)
        printf (" writeflag");
      if (horus)
        printf (" horus");
      printf ("\n");
    }

  if (horus)
    {
      sockfd = socket (PF_INET, SOCK_DGRAM, 0);
      assert (sockfd >= 0);
    }

  if (encrypt || decrypt)
    {
      memset (key, 0, sizeof (key));
      snprintf ((char *)key, sizeof (key), "initial key");
      cipher = aes_xts_init ();
    }

  rlevel = level;
  rboffset = boffset;
  rnblock = nblock;
  if (aggregate)
    {
      rlevel = alevel;
      rboffset = aboffset;
      rnblock = anblock;
    }

  if (horus && prefetch)
    {
      prefetch_key = (struct key_response_packet *)
        malloc (prefetch * sizeof (struct key_response_packet));
      assert (prefetch_key);
      memset (prefetch_key, 0,
              prefetch * sizeof (struct key_response_packet));
      memset (&sumnet, 0, sizeof (sumnet));
    }

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < rnblock; i++)
    {
      if (horus_verbose)
        printf ("for level: %lu block: %lu\n", rlevel, i);

      if (horus && prefetch)
        {
          gettimeofday (&startnet, NULL);
          if (i % prefetch == 0)
            horus_key_prefetch (filename, rlevel, i, MIN(prefetch,rnblock),
                                sockfd, &serv_addr[0], prefetch_key);
          rkey_len = ntohl (prefetch_key[i % prefetch].key_len);
          memcpy (rkey, prefetch_key[i % prefetch].key, rkey_len);
          gettimeofday (&endnet, NULL);
          timeval_sub (&endnet, &startnet, &resnet);
          timeval_merge (&sumnet, &resnet);
        }
      else if (horus)
        {
          /* key request */
          if (horus_verbose)
            printf ("request: K_%lu,%lu\n", rlevel, i);
          rkey_len = sizeof (rkey);
          horus_key_request ((char *)rkey, &rkey_len, filename,
                             rlevel, i, sockfd, &serv_addr[0]);
          /* set key */
          if (horus_verbose)
            printf ("got K_%lu,%lu = %s\n", rlevel, i,
                    print_key ((char *)rkey, HORUS_KEY_LEN));
        }

      /* calculate the leaf-level start and end block */
      startb = c.kht_block_size[rlevel] * i;
      endb = c.kht_block_size[rlevel] * (i + 1);
      if (length && length < endb * HORUS_BLOCK_SIZE)
        endb = (length / HORUS_BLOCK_SIZE) +
               (length % HORUS_BLOCK_SIZE ? 1 : 0);
      if (horus_verbose)
        printf ("  start: %lu end: %lu\n", startb, endb);

      for (j = startb; j < endb; j++)
        {
          if (horus_verbose)
            printf ("    for level: %lu block: %lu\n", leaf_level, j);

          if (horus && rlevel != leaf_level)
            {
              /* key request */
              key_len = sizeof (key);
              horus_block_key ((char *)key, &key_len, leaf_level, j,
                               (char *)rkey, rkey_len, rlevel, i,
                               c.kht_block_size);
              if (horus_verbose)
                printf ("    calculate leaf key: K_%lu,%lu = %s\n",
                        leaf_level, j, print_key ((char *)key, HORUS_KEY_LEN));
            }
          else
            {
              memcpy (key, rkey, HORUS_KEY_LEN);
              key_len = HORUS_KEY_LEN;
            }

          if (encrypt || decrypt)
            {
              memset (aeskey, 0, AES_KEYSIZE_128 * 2);
              memcpy (aeskey, key, HORUS_KEY_LEN);
              ret = aes_xts_setkey (cipher, aeskey, AES_KEYSIZE_128 * 2);
              assert (! ret);
              if (horus_verbose)
                printf ("    crypt key: = %s\n",
                        print_key ((char *)key, HORUS_KEY_LEN));
            }

          if (readflag)
            {
              if (horus_verbose)
                printf ("      read();\n");

              /* read */
              if (inputfd >= 0)
                ret = read (inputfd, block_storage, HORUS_BLOCK_SIZE);
              else
                ret = read (fd, block_storage, HORUS_BLOCK_SIZE);
              if (ret != HORUS_BLOCK_SIZE)
                {
                  if (horus_verbose)
                    printf ("read failed: %s\n", strerror (errno));
                }
              block = block_storage;

              if (decrypt)
                {
                  if (horus_verbose)
                    printf ("      decrypt by key: %s iv: %lu\n",
                            print_key ((char *)key, HORUS_KEY_LEN), j);

                  /* Use block id as IV */
                  memset (iv, 0, sizeof (iv));
                  *(unsigned long *)iv = j;
                  aes_xts_decrypt (cipher, block_data, block_storage,
                                   HORUS_BLOCK_SIZE, iv);
                  block = block_data;

                  total_decrypt++;
                }
              else
                {
                  memcpy (block_data, block_storage, HORUS_BLOCK_SIZE);
                }

              total_read++;

              if (horus_debug)
                print_block (block);
            }

          if (writeflag)
            {
              block = block_data;

              if (encrypt)
                {
                  if (horus_verbose)
                    printf ("      encrypt by key: %s iv: %lu.\n",
                            print_key ((char *)key, HORUS_KEY_LEN), j);

                  /* Use block id as IV */
                  memset (iv, 0, sizeof (iv));
                  *(unsigned long *)iv = j;
                  aes_xts_encrypt (cipher, block_storage, block_data,
                                   HORUS_BLOCK_SIZE, iv);
                  block = block_storage;

                  total_encrypt++;
                }

              if (horus_verbose)
                printf ("      write();\n");

              /* write */
              if (outputfd >= 0)
                ret = write (outputfd, block, HORUS_BLOCK_SIZE);
              else
                ret = write (fd, block, HORUS_BLOCK_SIZE);
              if (ret != HORUS_BLOCK_SIZE)
                {
                  if (horus_verbose)
                    printf ("write failed: %s\n", strerror (errno));
                }

              total_write++;
            }

          total++;
        }
    }

  if (benchmark)
    gettimeofday (&end, NULL);

  close (fd);
  if (inputfd >= 0)
    close (inputfd);
  if (outputfd >= 0)
    close (outputfd);
  if (sockfd >= 0)
    close (sockfd);

  if (prefetch)
    free (prefetch_key);

  if (benchmark)
    {
      unsigned long long bytes;
      double time, Bps, IOps;
      double timenet;

      timeval_sub (&end, &start, &res);
      time = res.tv_sec + res.tv_usec * 0.000001;
      bytes = total * HORUS_BLOCK_SIZE;
      Bps = bytes / time;
      IOps = total / time;

      printf ("read/write: read: %lu decrypt: %lu encrypt: %lu write: %lu\n",
              total_read, total_decrypt, total_encrypt, total_write);
      printf ("horus: reqlevel: %lu #reqkeys %lu\n", rlevel, rnblock);
      printf ("time: length %llu nblock: %lu time %f secs\n",
              length, total, time);
      if (prefetch)
        {
          timenet = sumnet.tv_sec + sumnet.tv_usec * 0.000001;
          printf ("prefetch: #keys: %d time: %f secs\n", prefetch, timenet);
        }
      printf ("I/Ops: %f I/Ops Bps: %f Bps\n", IOps, Bps);
    }

  return 0;
}



