
#include <horus.h>
#include <horus_attr.h>

#include <getopt.h>

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

char *progname;
int debug = 0;

const char *optstring = "dh";
const char *optusage = "\
-d, --debug    Turn on debugging mode\n\
-h, --help     Display this help and exit\n\
";
const struct option longopts[] = {
  { "debug", no_argument, NULL, 'd' },
  { "help",  no_argument, NULL, 'h' },
  { NULL,    0,           NULL, 0   }
};

const char *cmdusage = "\
  show            Show horus configuration for the file.\n\
  master-key      Set master key. args: <key>\n\
  kht-block-sizes Set block sizes for key hash tree. args: <blocksize>...\n\
  allow           Allow client access. args: <ipaddr> <start> <end>\n\
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
horus_file_cmd_show (int fd)
{
  int i, ret;
  struct horus_file_config c;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      fprintf (stderr, "no horus config found.\n");
      return ret;
    }

  printf ("%-24s%s\n", "horus master key:", c.master_key);
  printf ("%-24s", "horus kht block sizes:");
  for (i = 0; i < HORUS_MAX_KHT_DEPTH &&
       c.kht_block_size[i]; i++)
    printf (" %u", c.kht_block_size[i]);
  printf ("\n");
  printf ("horus client range:\n");
  for (i = 0; i < HORUS_MAX_CLIENT_ENTRY &&
       c.client_range[i].ipaddr.s_addr != INADDR_ANY; i++)
    {
      char buf[32];
      inet_ntop (AF_INET, &c.client_range[i].ipaddr, buf, sizeof (buf));
      printf ("  %s: %u - %u\n", buf,
              c.client_range[i].start, c.client_range[i].end);
    }

  return ret;
}

int
horus_file_cmd_master_key (int fd, char *key)
{
  return horus_set_master_key (fd, key);
}

int
main (int argc, char **argv)
{
  int i, ch;
  int fd;
  char *target, *cmd, *arg;

  progname = (1 ? "horus-file" : argv[0]);

  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'd':
          debug++;
          break;
        default:
          usage ();
          break;
        }
    }
  argc -= optind;
  argv += optind;

  target = cmd = arg = NULL;
  if (argc > 0)
    target = argv[0];
  if (argc > 1)
    cmd = argv[1];
  if (argc > 2)
    arg = argv[2];

  if (debug)
    {
      if (target)
        printf ("target: %s\n", target);
      if (cmd)
        printf ("cmd:    %s\n", cmd);
      if (arg)
        printf ("cmd:    %s\n", arg);
    }

  fd = open (target, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "no such file: %s\n", target);
      return -1;
    }

  printf ("file: %s\n", target);
  if (cmd && ! strcmp (cmd, "show"))
    horus_file_cmd_show (fd);
  if (cmd && ! strcmp (cmd, "master-key") && arg)
    horus_file_cmd_master_key (fd, arg);

  close (fd);
  return 0;
}



