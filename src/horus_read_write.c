#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <horus_attr.h>
#include <horus_stats.h>
#include <benchmark.h>
#include "timeval.h"

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern int horus_debug;
extern int horus_verbose;

int benchmark = 0;
int aggregate = 0;

const char *optstring = "hvdbeus:rwf:l:a:n:";
const char *optusage = "\
-h, --help        Display this help and exit\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-b, --benchmark   Turn on benchmarking\n\
-e, --encrypt     Turn on encrypting/decrypting\n\
-u, --horus       Turn on Horus mode\n\
-s, --server      Specify server IP address A.B.C.D[:P] (default: %s:%d)\n\
-a, --aggregate   Specify the level of aggregation of request range keys\n\
-r, --read        Do read\n\
-w, --write       Do write\n\
-f, --file        Specify the file name\n\
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
  { "horus",      no_argument,        NULL, 'u' },
  { "server",     required_argument,  NULL, 's' },
  { "aggregate",  required_argument,  NULL, 'a' },
  { "read",       no_argument,        NULL, 'r' },
  { "write",      no_argument,        NULL, 'w' },
  { "file",       required_argument,  NULL, 'f' },
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

int
main (int argc, char **argv)
{
  int fd, ret, ch;
  unsigned long i, j;
  unsigned long start, end;
  int ncount = 0;
  unsigned long long offset, length, size;
  char *filename = NULL;
  char *endptr;
  int encrypt, horus, readflag, writeflag;
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

  memset (&serv_addr, 0, sizeof (serv_addr));
  horus = encrypt = aggregate = 0;
  readflag = writeflag = 0;

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
        case 'r':
          readflag++;
          break;
        case 'w':
          writeflag++;
          break;
        case 'f':
          filename = optarg;
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
      printf ("specify filename to read/write\n");
      exit (1);
    }

  if (nservers == 0)
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
      if (benchmark)
        printf ("benchmark: ");
      printf ("level: %lu boffset: %lu nblock: %lu\n",
              level, boffset, nblock);
      if (aggregate)
        printf ("alevel: %lu aboffset: %lu anblock: %lu\n",
                alevel, aboffset, anblock);
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

  for (i = 0; i < rnblock; i++)
    {
      if (horus_verbose)
        printf ("for level: %lu block: %lu\n", rlevel, i);

      if (horus)
        {
          /* key request */
          printf ("request: K_%lu,%lu\n", rlevel, i);
          printf ("receive: K_%lu,%lu\n", rlevel, i);
        }

      /* calculate the leaf-level start and end block */
      start = c.kht_block_size[rlevel] * i;
      end = c.kht_block_size[rlevel] * (i + 1);
      if (length && length < end * HORUS_BLOCK_SIZE)
        end = (length / HORUS_BLOCK_SIZE) +
              (length % HORUS_BLOCK_SIZE ? 1 : 0);
      if (horus_verbose)
        printf ("  start: %lu end: %lu\n", start, end);

      for (j = start; j < end; j++)
        {
          if (horus_verbose)
            printf ("    for level: %lu block: %lu\n", leaf_level, j);

          if (horus && rlevel != leaf_level)
            {
              if (horus_verbose)
                printf ("    calculate leaf key\n");
              //horus_block_key ();
            }

          if (readflag)
            {
              if (encrypt)
                {
                  if (horus_verbose)
                    printf ("      read_decrypt(key);\n");
                  //horus_read ();
                }
              else
                {
                  if (horus_verbose)
                    printf ("      read();\n");
                  //read ();
                }
            }

          if (writeflag)
            {
              if (encrypt)
                {
                  if (horus_verbose)
                    printf ("      write_encrypt(key);\n");
                  //horus_write ();
                }
              else
                {
                  if (horus_verbose)
                    printf ("      write();\n");
                  //write ();
                }
            }
        }
    }

  close (fd);

  return 0;
}



