
#include <horus.h>
#include <horus_attr.h>

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
int debug = 0;

const char *optstring = "vdh";
const char *optusage = "\
-v, --verbose  Turn on verbose mode\n\
-d, --debug    Turn on debugging mode\n\
-h, --help     Display this help and exit\n\
";
const struct option longopts[] = {
  { "verbose", no_argument, NULL, 'v' },
  { "debug", no_argument, NULL, 'd' },
  { "help",  no_argument, NULL, 'h' },
  { NULL,    0,           NULL, 0   }
};

const char *cmdusage = "\
  show            Show horus configuration for the file.\n\
  master-key      Set master key. args: <key>\n\
  kht-block-sizes Set block sizes for key hash tree. args: <blocksize>...\n\
  client add      Allow client access. args: <ipaddr> <start> <end>\n\
  client clear    Clear client access.\n\
";

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"

void
usage ()
{
  printf ("Usage : %s [OPTION...] <filename> <command> [<arg>]\n", progname);
  printf ("\n");
  printf ("options are\n%s\n", optusage);
  printf ("commands are\n%s\n", cmdusage);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

int
horus_client_range_print (char *buf, int size, struct horus_client_range *p)
{
  int ret;
  char addr[32];
  inet_ntop (AF_INET, &p->prefix, addr, sizeof (addr));
  ret = snprintf (buf, size, "%s/%d: %u - %u",
                  addr, p->prefixlen, p->start, p->end);
  return ret;
}

int
horus_file_cmd_show (int fd, char *target)
{
  int i, ret;
  struct horus_file_config c;
  char buf[64];
  struct horus_client_range *p;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      fprintf (stderr, "no horus config found.\n");
      return ret;
    }

  printf ("* the unit is %d-bytes block.\n", HORUS_BLOCK_SIZE);

  printf ("file: %s\n", target);
  printf ("%-24s %s\n", "  master key:", c.master_key);
  printf ("%-24s", "  kht block sizes:");
  for (i = 0; i < HORUS_MAX_KHT_DEPTH &&
       c.kht_block_size[i]; i++)
    printf (" %u", c.kht_block_size[i]);
  printf ("\n");

  printf ("%-24s\n", "  client range:");
  if (debug)
    {
      for (i = 0; i < HORUS_MAX_CLIENT_ENTRY; i++)
        {
          p = &c.client_range[i];
          horus_client_range_print (buf, sizeof (buf), p);
          printf ("    client[%d]: %s\n", i, buf);
        }
    }
  else
    {
      for (i = 0; i < HORUS_MAX_CLIENT_ENTRY &&
           ! IS_HORUS_CLIENT_RANGE_EMPTY (&c.client_range[i]); i++)
        {
          p = &c.client_range[i];
          horus_client_range_print (buf, sizeof (buf), p);
          printf ("    client[%d]: %s\n", i, buf);
        }
    }

  return ret;
}

int
horus_file_cmd_master_key (int fd, int argc, char **argv)
{
  char *key = NULL;

  if (argc > 0)
    {
      key = argv[0];
      argc--;
      argv++;
    }

  if (! key)
    {
      fprintf (stderr, "specify master-key.\n");
      return -1;
    }

  horus_set_master_key (fd, key);

  return 0;
}

int
horus_file_cmd_kht_block_sizes (int fd, int argc, char **argv)
{
  int i;
  unsigned int kht_block_sizes[HORUS_MAX_KHT_DEPTH];
  unsigned int nblock;
  unsigned long maxdepth, branch;
  unsigned long long size, max, upper;
  char *endptr;

  maxdepth = HORUS_MAX_KHT_DEPTH;
  if (argc > maxdepth)
    {
      fprintf (stderr, "maximum depth of %lu is allowed (%d specified).\n",
               maxdepth, argc);
      return -1;
    }

  memset (kht_block_sizes, 0, sizeof (kht_block_sizes));

  /* for each level's block size */
  for (i = 0; i < argc; i++)
    {
      size = canonical_byte_size (argv[i], &endptr);
      if (*endptr != '\0')
        {
          fprintf (stderr, "invalid \'%c\' in "
                   "size[%d]: %s\n", *endptr, i, argv[i]);
          return -1;
        }

      if (size % HORUS_BLOCK_SIZE)
        {
          fprintf (stderr, "size not multiple of block size %d in "
                   "size[%d]: %s\n", HORUS_BLOCK_SIZE, i, argv[i]);
          return -1;
        }

      max = (unsigned long long) ULONG_MAX * HORUS_BLOCK_SIZE;
      if (size > max)
        {
          fprintf (stderr, "size %llu exceeds the limit %llu in "
                   "size[%d]: %s\n", size, max, i, argv[i]);
          return -1;
        }

      nblock = (unsigned int) (size / HORUS_BLOCK_SIZE);
      upper = (i == 0 ? nblock : kht_block_sizes[i - 1]);
      if (upper % nblock)
        {
          fprintf (stderr, "upper is not dividable by current in "
                   "size[%d]: %s\n", i, argv[i]);
          return -1;
        }

      branch = upper / nblock;

      kht_block_sizes[i] = (unsigned int) nblock;

      //if (debug)
        printf ("kht_block_sizes[%2d] = %10u (blks) "
                "(%13llu bytes) (%lu branch)\n",
                i, nblock, size, branch);
    }

  for (i = 0; i < argc; i++)
    horus_set_kht_block_size (fd, i, kht_block_sizes[i]);

  return 0;
}

int
horus_file_cmd_add_client_range (int fd, char *prefix,
                                 char *start, char *end)
{
  struct in_addr vprefix;
  int prefixlen = 32;
  unsigned long long vstart, vend, max;
  char *p;
  char *endptr;
  int ret;
  unsigned long nb_start, nb_end;

  p = index (prefix, '/');
  if (p)
    {
      *p++ = '\0';
      prefixlen = strtol (p, &endptr, 0);
      if (*endptr != '\0')
        {
          fprintf (stderr, "invalid prefixlen: %s\n", p);
          return -1;
        }
    }

  ret = inet_pton (AF_INET, prefix, &vprefix);
  if (ret != 1)
    {
      fprintf (stderr, "parse failed in inet_pton(): %s\n", prefix);
      return -1;
    }

  max = (unsigned long long) (0xffffffffUL * HORUS_BLOCK_SIZE);

  vstart = canonical_byte_size (start, &endptr);
  if (*endptr != '\0')
    {
      fprintf (stderr, "invalid \'%c\' in start: %s\n", *endptr, start);
      return -1;
    }
  if (vstart > max)
    {
      fprintf (stderr, "start %llu exceeds the limit %llu in "
               "%s\n", vstart, max, start);
      return -1;
    }
  nb_start = (unsigned int) (vstart / HORUS_BLOCK_SIZE);

  vend = canonical_byte_size (end, &endptr);
  if (*endptr != '\0')
    {
      fprintf (stderr, "invalid \'%c\' in end: %s\n", *endptr, end);
      return -1;
    }
  if (vend > max)
    {
      fprintf (stderr, "end %llu exceeds the limit %llu in %s\n",
               vend, max, end);
      fprintf (stderr, "end adjusted to the limit %llu\n", max);
      vend = max;
    }
  nb_end = (unsigned int) (vend / HORUS_BLOCK_SIZE);

  {
    char buf[64];
    inet_ntop (AF_INET, &vprefix, buf, sizeof (buf));
    printf ("prefix = %s/%d start = %llu end = %llu\n",
            buf, prefixlen, vstart, vend);
  }

  horus_add_client_range (fd, &vprefix, prefixlen, nb_start, nb_end);

  return 0;
}

int
horus_file_cmd_clear_client_range (int fd)
{
  horus_clear_client_range (fd);
  return 0;
}

int
horus_file_cmd_client_range (int fd, int argc, char **argv)
{
  char *cmd;
  int ret;

  cmd = NULL;
  if (argc > 0)
    {
      cmd = argv[0];
      argc--;
      argv++;
    }

  if (! cmd)
    {
      fprintf (stderr, "specify client sub-command.\n");
      return -1;
    }

  if (! strcmp (cmd, "add"))
    {
      if (argc < 3)
        {
          fprintf (stderr, "Usage: %s ", progname);
          fprintf (stderr, "client add <IP prefix> <start> <end>\n");
          return -1;
        }

      ret = horus_file_cmd_add_client_range (fd, argv[0], argv[1], argv[2]);
    }
  else if (! strcmp (cmd, "clear"))
    {
      ret = horus_file_cmd_clear_client_range (fd);
    }
  else
    {
      fprintf (stderr, "no such command: %s\n", cmd);
      return -1;
    }

  return ret;
}

int
horus_file_cmd_delete (int fd)
{
  return horus_delete_file_config (fd);
}

int
main (int argc, char **argv)
{
  int ch;
  int fd;
  char *target, *cmd;

  progname = (1 ? "horus-file" : argv[0]);

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
        default:
          usage ();
          break;
        }
    }
  argc -= optind;
  argv += optind;

  target = NULL;
  if (argc > 0)
    {
      target = argv[0];
      argc--;
      argv++;
    }

  cmd = NULL;
  if (argc > 0)
    {
      cmd = argv[0];
      argc--;
      argv++;
    }

  if (debug)
    {
      if (target)
        printf ("target: %s\n", target);
      if (cmd)
        printf ("cmd:    %s\n", cmd);
    }

  if (! target)
    {
      fprintf (stderr, "specify target filename.\n");
      return -1;
    }

  if (! cmd)
    {
      fprintf (stderr, "specify command.\n");
      return -1;
    }

  fd = open (target, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "no such file: %s\n", target);
      return -1;
    }

  if (! strcmp (cmd, "show"))
    horus_file_cmd_show (fd, target);
  else if (! strcmp (cmd, "master-key"))
    horus_file_cmd_master_key (fd, argc, argv);
  else if (! strcmp (cmd, "kht-block-sizes"))
    horus_file_cmd_kht_block_sizes (fd, argc, argv);
  else if (! strcmp (cmd, "client"))
    horus_file_cmd_client_range (fd, argc, argv);
  else if (! strcmp (cmd, "delete"))
    horus_file_cmd_delete (fd);
  else
    fprintf (stderr, "no such commands: %s\n", cmd);

  close (fd);
  return 0;
}

