#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <benchmark.h>
#include <getopt.h>

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;
extern horus_debug;

const char *optstring = "bdh";
const char *optusage = "\
-b, --benchmark  Turn on benchmarking\n\
-d, --debug      Turn on debugging mode\n\
-h, --help       Display this help\n\
";

const struct option longopts[] = {
  {"benchmark", no_argument, NULL, 'b'},
  {"debug", no_argument, NULL, 'd'},
  {"help", no_argument, NULL, 'h'},
  {NULL, 0, NULL, 0}
};


void
usage ()
{
  printf ("Usage : %s [OPTION...] KDS_IP filename x y\n", progname);
  printf ("\n");
  printf ("options are \n%s\n", optusage);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}


int
client_sendrecv (int fd, struct sockaddr_in srv_addr,
                 const key_request_packet * kreq, key_response_packet * kresp)
{
  int ret = -1, addrlen;

  assert ((kreq != NULL) && (kresp != NULL));
  ret = sendto (fd, kreq, sizeof (key_request_packet), 0,
                (struct sockaddr *) &srv_addr, sizeof (srv_addr));
  assert (ret == sizeof (key_request_packet));
  do
    {
      addrlen = sizeof (srv_addr);
      ret = recvfrom (fd, kresp, sizeof (key_request_packet), 0,
                      (struct sockaddr *) &srv_addr, &addrlen);
    }
  while (ret <= 0);
  assert (ret == sizeof (key_response_packet));
  return 0;
}

int
main (int argc, char **argv)
{
  int fd;
  int ret, ch, benchmark = 0, i;
  struct sockaddr_in srv_addr;
  struct key_request_packet kreq;
  struct key_response_packet kresp;

  progname = (1 ? "kds_client" : argv[0]);
  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'b':
          benchmark++;
          break;
        case 'd':
          horus_debug++;
          break;
        default:
          usage ();
          break;
        }
    }
  argc -= optind;
  argv += optind;

  if (argc != 4)
    usage ();

  strcpy (kreq.filename, argv[1]);
  kreq.x = htonl (atoi (argv[2]));
  kreq.y = htonl (atoi (argv[3]));

  memset (&srv_addr, 0, sizeof (srv_addr));
  srv_addr.sin_family = PF_INET;
  srv_addr.sin_port = htons (HORUS_KDS_SERVER_PORT);
  inet_pton (PF_INET, argv[0], &srv_addr.sin_addr);

  if (benchmark)
    start_clock ();
  fd = socket (PF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
    {
      fprintf (stderr, "Unable to open socket!\n");
      exit (1);
    }

  client_sendrecv (fd, srv_addr, &kreq, &kresp);
  if (ntohl (kresp.err) == 0)
    {
      printf ("key = %s\n", print_key (kresp.key, ntohl (kresp.key_len)));
    }
  else
    {
      int kresperr = (int) ntohl (kresp.err);
      fprintf (stderr, "err = %d: %s\n",
               kresperr, strerror (kresperr));
    }
  if (benchmark)
    {
      end_clock ();
      printf ("time = ");
      print_last_clock_diff ();
    }
  printf ("\n");
}
