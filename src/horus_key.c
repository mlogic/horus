
#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>

#include <getopt.h>
#include <assert.h>
#include <limits.h>
#include "timeval.h"

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;
int benchmark = 0;

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern int horus_debug;
extern int horus_verbose;

const char *optstring = "bvdhx:y:";
const char *optusage = "\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-h, --help        Display this help and exit\n\
-x, --key-x       Specify x to calculate key K_x,y\n\
-y, --key-y       Specify y to calculate key K_x,y\n\
-b, --benchmark   Turn on benchmark mode: <depth> <branch> <blocksize> <filesize>\n\
";
const struct option longopts[] = {
  { "benchmark",  no_argument,        NULL, 'b' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "help",       no_argument,        NULL, 'h' },
  { "key-x",      required_argument,  NULL, 'x' },
  { "key-y",      required_argument,  NULL, 'y' },
  { NULL,         0,                  NULL,  0  }
};

void
usage ()
{
  printf ("Usage : %s [OPTION...] <filename> [<offset>]\n", progname);
  printf ("\n");
  printf ("options are\n%s\n", optusage);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

void
horus_key_calc_benchmark (struct horus_file_config *c, int argc, char **argv)
{
  int i, ret;
  char key[64];
  size_t key_len;
  unsigned long long x, y, nblocks;
  int depth = 0;
  int branch = 0;
  unsigned long long blocksize = 0;
  unsigned long long filesize = 2 * 1024 * 1024 * 1024LLU;
  struct timeval start, end, res;
  double time;
  char buf[128];

  if (argc > 0)
    {
      depth = atoi (argv[0]);
      argc--;
      argv++;
    }
  if (argc > 0)
    {
      branch = atoi (argv[0]);
      argc--;
      argv++;
    }
  if (argc > 0)
    {
      blocksize = canonical_byte_size (argv[0], NULL);
      argc--;
      argv++;
    }
  if (argc > 0)
    {
      filesize = canonical_byte_size (argv[0], NULL);
      argc--;
      argv++;
    }

  if (depth || branch || blocksize)
    {
      if (! depth)
        depth = 10;
      if (! branch)
        branch = 4;
      if (! blocksize)
        blocksize = 4096;

      memset (c->kht_block_size, 0, sizeof (c->kht_block_size));
      i = depth;
      while (i > 0)
        {
          i--;
          if (i == depth - 1)
            c->kht_block_size[i] = 1;
          else
            c->kht_block_size[i] = c->kht_block_size[i + 1] * branch;
        }
    }
  else
    {
       depth = 0;
       for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
         if (c->kht_block_size[i])
           depth++;
    }

  blocksize = HORUS_BLOCK_SIZE;

  x = depth - 1;
  nblocks = filesize / blocksize;

  printf ("depth: %d branch: %d blocksize: %llu "
          "filesize: %llu #blocks: %llu\n",
          depth, branch, blocksize, filesize, nblocks);

  printf ("kht block sizes:");
  for (i = 0; i < HORUS_MAX_KHT_DEPTH &&
       c->kht_block_size[i]; i++)
    printf (" %u", c->kht_block_size[i]);
  printf ("\n");

  gettimeofday (&start, NULL);

  for (y = 0; y < nblocks; y++)
    {
      key_len = sizeof (key);
      ret = horus_key_by_master (key, &key_len, x, y,
                                 c->master_key, strlen (c->master_key),
                                 c->kht_block_size);
      assert (ret == 0);
    }

  gettimeofday (&end, NULL);

  timeval_sub (&end, &start, &res);
  time = res.tv_sec + res.tv_usec * 0.000001;
  snprint_timeval (buf, sizeof (buf), &res);

  printf ("depth: %d branch: %d blocksize: %llu "
          "filesize: %llu #blocks: %llu %s secs ( %f q/s\n",
          depth, branch, blocksize, filesize, nblocks, buf, nblocks/time);
}

int
main (int argc, char **argv)
{
  int ch;
  int fd;
  char *target;
  unsigned long long offset;
  struct horus_file_config c;

  int i, ret;
  int x, y;
  int keyx, keyy;
  char *strx, *stry;

  char key[SHA_DIGEST_LENGTH * 2 + 1];
  size_t key_len;

  progname = (1 ? "horus-key" : argv[0]);

  strx = stry = NULL;
  keyx = keyy = 0;

  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'b':
          benchmark++;
          break;
        case 'v':
          horus_verbose++;
          break;
        case 'd':
          horus_debug++;
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
  if (! benchmark && argc > 0)
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

  if (horus_verbose)
    printf ("offset = %llu, master = %s(%d).\n",
            offset, print_key (c.master_key, (int) strlen (c.master_key)),
            (int) strlen (c.master_key));

  if (horus_verbose)
    {
      for (i = 0; i < HORUS_MAX_KHT_DEPTH && c.kht_block_size[i]; i++)
        printf ("kht_block_size[%d] = %u\n", i, c.kht_block_size[i]);
    }

  if (benchmark)
    {
      horus_key_calc_benchmark (&c, argc, argv);
      close (fd);
      return 0;
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

  if (horus_verbose)
    printf ("calculate K%d,%d.\n", x, y);

  key_len = sizeof (key);
  ret = horus_key_by_master (key, &key_len, x, y,
                             c.master_key, strlen (c.master_key),
                             c.kht_block_size);

  if (ret == 0)
    printf ("K_%d,%d = %s(%d) [%llu-%llu]\n",
      x, y, print_key (key, key_len), (int) key_len,
      (unsigned long long) c.kht_block_size[x] * HORUS_BLOCK_SIZE * y,
      (unsigned long long) c.kht_block_size[x] * HORUS_BLOCK_SIZE * (y + 1) - 1);

  close (fd);

  return 0;
}



