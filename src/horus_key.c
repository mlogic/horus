
#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>

#include <getopt.h>
#include <assert.h>
#include <limits.h>

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

char *progname;
int verbose = 0;
//int debug = 0;

const char *optstring = "vdhx:y:";
const char *optusage = "\
-v, --verbose  Turn on verbose mode\n\
-d, --debug    Turn on debugging mode\n\
-h, --help     Display this help and exit\n\
-x, --key-x    Specify x to calculate key K_x,y\n\
-y, --key-y    Specify y to calculate key K_x,y\n\
";
const struct option longopts[] = {
  { "verbose", no_argument, NULL, 'v' },
  { "debug", no_argument, NULL, 'd' },
  { "help",  no_argument, NULL, 'h' },
  { "key-x", required_argument, NULL, 'x' },
  { "key-y", required_argument, NULL, 'y' },
  { NULL,    0,           NULL, 0   }
};

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"

void
usage ()
{
  printf ("Usage : %s [OPTION...] <filename> [<offset>]\n", progname);
  printf ("\n");
  printf ("options are\n%s\n", optusage);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

int
main (int argc, char **argv)
{
  int ch;
  int fd;
  char *target, *cmd;
  unsigned long long offset;
  struct horus_file_config c;

  int i, ret;
  int x, y;
  int keyx, keyy;
  char *strx, *stry;

  char parent[SHA_DIGEST_LENGTH * 2 + 1];
  char key[SHA_DIGEST_LENGTH];
  int parent_len, key_len;

  progname = (1 ? "horus-key" : argv[0]);

  strx = stry = NULL;
  keyx = keyy = 0;

  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'd':
          debug++;
          break;
        case 'v':
          verbose++;
          break;
        case 'x':
          strx = optarg;
          keyx = atoi (strx);
          break;
        case 'y':
          stry = optarg;
          keyy = atoi (stry);
          break;
        default:
          usage ();
          break;
        }
    }
  argc -= optind;
  argv += optind;

  if (strx || stry)
    {
      if (! strx)
        {
          fprintf (stderr, "x was not specified while y was.\n");
          return -1;
        }
      if (! stry)
        {
          fprintf (stderr, "y was not specified while x was.\n");
          return -1;
        }
    }

  target = NULL;
  if (argc > 0)
    {
      target = argv[0];
      argc--;
      argv++;
    }

  /* Byte offset */
  offset = 0;
  if (argc > 0)
    {
      char *endptr;
      offset = canonical_byte_size (argv[0], &endptr);
      if (*endptr != '\0')
        {
          fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, argv[0]);
          return -1;
        }
      argc--;
      argv++;
    }

  if (! target)
    {
      fprintf (stderr, "specify target filename.\n");
      return -1;
    }

  fd = open (target, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "no such file: %s\n", target);
      return -1;
    }

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      fprintf (stderr, "no horus config found for %s.\n", target);
      fprintf (stderr, "first set by horus-file.\n");
      close (fd);
      return ret;
    }

  /* Master key */
  memset (parent, 0, sizeof (parent));
  snprintf (parent, sizeof (parent), "%s", c.master_key);
  parent_len = strlen (parent);

  if (verbose)
    printf ("offset = %llu, master = %s(%d).\n",
            offset, print_key (parent, parent_len), parent_len);

  if (verbose)
    {
      for (i = 0; i < HORUS_MAX_KHT_DEPTH && c.kht_block_size[i]; i++)
        printf ("kht_block_size[%d] = %u\n", i, c.kht_block_size[i]);
    }

  if (strx)
    {
      x = keyx;
      y = keyy;
    }
  else
    {
      x = 0;
      for (i = 0; i < HORUS_MAX_KHT_DEPTH && c.kht_block_size[i]; i++)
        x = i;
      if (c.kht_block_size[x] == 0)
        {
          fprintf (stderr, "invalid kht_block_sizes for %s.\n", target);
          fprintf (stderr, "first set by horus-file.\n");
          close (fd);
          return ret;
        }
      y = offset / (c.kht_block_size[x] * HORUS_BLOCK_SIZE);
    }

  if (verbose)
    printf ("calculate K%d,%d.\n", x, y);

  horus_key_by_master (key, &key_len, x, y, parent, parent_len);

  printf ("K%d,%d = %s(%d) [%llu-%llu]\n",
    x, y, print_key (key, key_len), key_len,
    (unsigned long long) c.kht_block_size[x] * HORUS_BLOCK_SIZE * y,
    (unsigned long long) c.kht_block_size[x] * HORUS_BLOCK_SIZE * (y + 1) - 1);

  close (fd);

  return 0;
}



