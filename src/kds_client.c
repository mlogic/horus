#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <benchmark.h>
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

const char *optstring = "bvdhs:p:x:y:o:l:";
const char *optusage = "\
-b, --benchmark   Turn on benchmarking\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-h, --help        Display this help and exit\n\
-s, --server      Specify server IP address (default: %s)\n\
-p, --port        Specify server UDP port (default: %d)\n\
-x, --key-x       Specify x to calculate key K_x,y\n\
-y, --key-y       Specify y to calculate key K_x,y\n\
-o, --offset      Specify file offset to access\n\
-l, --length      Specify size of the range to access (from offset)\n\
";

const struct option longopts[] = {
  { "benchmark",  no_argument,        NULL, 'b' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "help",       no_argument,        NULL, 'h' },
  { "key-x",      required_argument,  NULL, 'x' },
  { "key-y",      required_argument,  NULL, 'y' },
  { "offset",     required_argument,  NULL, 'o' },
  { "length",     required_argument,  NULL, 'l' },
  { NULL,         0,                  NULL,  0  }
};

void
usage ()
{
  printf ("Usage : %s [OPTION...] <server_addr> <filename>\n", progname);
  printf ("\n");
  printf ("options are \n");
  printf (optusage, HORUS_KDS_SERVER_ADDR, HORUS_KDS_SERVER_PORT);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

int
client_sendrecv (int fd, struct sockaddr_in *srv_addr,
                 const key_request_packet* kreq,
                 key_response_packet *kresp)
{
  int ret = -1;
  socklen_t addrlen;

  assert ((kreq != NULL) && (kresp != NULL));
  ret = sendto (fd, kreq, sizeof (key_request_packet), 0,
                (struct sockaddr *) srv_addr, sizeof (struct sockaddr_in));
  assert (ret == sizeof (key_request_packet));
  do
    {
      addrlen = sizeof (struct sockaddr_in);
      ret = recvfrom (fd, kresp, sizeof (key_response_packet), 0,
                      (struct sockaddr *) srv_addr, &addrlen);
    }
  while (ret <= 0);
  assert (ret == sizeof (key_response_packet));
  return 0;
}

int
main (int argc, char **argv)
{
  int fd;
  int ret, ch, i;
  struct in_addr sin_addr;
  u_int16_t port;
  struct sockaddr_in srv_addr;
  struct key_request_packet kreq;
  struct key_response_packet kresp;
  int keyx, keyy;
  unsigned long long off, len;
  char *server = NULL, *filename = NULL;
  char *endptr;
  struct timeval start, end, res;
  int kresperr, krespkeylen;
  double time;

  unsigned long long stat_success, stat_einval, stat_eaccess;

  keyx = keyy = 0;
  off = len = 0;
  stat_success = stat_einval = stat_eaccess = 0;

  server = HORUS_KDS_SERVER_ADDR;
  ret = inet_pton (AF_INET, server, &sin_addr);
  assert (ret == 1);
  port = HORUS_KDS_SERVER_PORT;

  progname = (1 ? "kds_client" : argv[0]);
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
        case 's':
          server = optarg;
          ret = inet_pton (AF_INET, server, &sin_addr);
          if (ret != 1)
            {
              fprintf (stderr, "invalid server address: %s\n", optarg);
              return -1;
            }
          break;
        case 'p':
          port = (u_int16_t) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'x':
          keyx = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'y':
          keyy = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'o':
          off = canonical_byte_size (optarg, &endptr);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'l':
          len = canonical_byte_size (optarg, &endptr);
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

  if (! filename)
    usage ();

  strcpy (kreq.filename, filename);

  memset (&srv_addr, 0, sizeof (srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons (port);
  srv_addr.sin_addr = sin_addr;

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "Unable to open socket!\n");
      exit (1);
    }

  if (keyx || keyy)
    {
      kreq.x = htonl (keyx);
      kreq.y = htonl (keyy);
      client_sendrecv (fd, &srv_addr, &kreq, &kresp);

      kresperr = (int) ntohl (kresp.err);
      krespkeylen = (int) ntohl (kresp.key_len);
      printf ("err = %d: %s\n", kresperr, strerror (kresperr));
      if (! kresperr)
        printf ("key_%d,%d = %s\n", keyx, keyy,
                print_key (kresp.key, krespkeylen));
    }
  else
    {
      unsigned long offblock, nblock;
      unsigned long leaf_level = 9;

      keyx = leaf_level;
      kreq.x = htonl (keyx);

      offblock = (off ? off / HORUS_BLOCK_SIZE : 0);
      nblock = (len ? len / HORUS_BLOCK_SIZE : 1);

      if (horus_verbose)
        printf ("block offset %lu length %lu\n", offblock, nblock);

      if (benchmark)
        gettimeofday (&start, NULL);

      for (i = 0; i < nblock; i++)
        {
          keyy = offblock + i;
          kreq.y = htonl (keyy);
          client_sendrecv (fd, &srv_addr, &kreq, &kresp);

          kresperr = (int) ntohl (kresp.err);
          krespkeylen = (int) ntohl (kresp.key_len);

          switch (kresperr)
            {
            case 0:
              stat_success++;
              break;
            case EINVAL:
              stat_einval++;
              break;
            case EACCES:
              stat_eaccess++;
              break;
            default:
              break;
            }

          if (! benchmark)
            {
              printf ("err = %d: %s\n", kresperr, strerror (kresperr));
              if (! kresperr)
                printf ("key_%d,%d = %s\n", keyx, keyy,
                         print_key (kresp.key, krespkeylen));
            }
        }

      if (benchmark)
        gettimeofday (&end, NULL);

      printf ("key total: %lu success: %llu inval: %llu eaccess: %llu\n",
              nblock, stat_success, stat_einval, stat_eaccess);

      if (benchmark)
        {
          timeval_sub (&end, &start, &res);
          time = res.tv_sec + res.tv_usec * 0.000001;
          printf ("%lu keys in %f secs ( %f q/s\n",
                  nblock, time, nblock/time);
        }
    }

  return 0;
}

