
#include <horus.h>
#include <horus_attr.h>

#include <getopt.h>

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

char *progname;

const char *optcommands = "[-d] [-h]";
const char *optstring = "dh";
const struct option longopts[] = {
  {"debug", no_argument, NULL, 'd' },
  {"help",  no_argument, NULL, 'h' },
  {NULL,    0,           NULL, 0   }
};

void
usage ()
{
  printf ("%s %s\n", progname, optcommands);
  exit (0);
}

int
main (int argc, char **argv)
{
  int ch;

  progname = argv[0];

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

  printf ("okay.\n");

  return 0;
}



