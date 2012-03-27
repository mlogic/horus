#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>
#include "kds.h"
#include <getopt.h>
#include <benchmark.h>

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;
int benchmark;

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
  printf ("Usage : %s [OPTION...]\n", progname);
  printf ("\n");
  printf ("options are \n%s\n", optusage);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}



static int num_threads = 0;
pthread_mutex_t kds_client_th = PTHREAD_MUTEX_INITIALIZER;
static void
increment_thread_count (void)
{
  pthread_mutex_lock (&kds_client_th);
  num_threads++;
  pthread_mutex_unlock (&kds_client_th);
}

static void
decrement_thread_count (void)
{
  pthread_mutex_lock (&kds_client_th);
  num_threads--;
  pthread_mutex_unlock (&kds_client_th);
}

static int
thread_count (void)
{
  int val;

  pthread_mutex_lock (&kds_client_th);
  val = num_threads;
  pthread_mutex_unlock (&kds_client_th);
  return val;
}

static int
kds_get_client_key (struct in_addr *client, key_request_packet * kreq,
                    key_response_packet * kresp)
{
  int fd, ret;
  unsigned int client_allowed_sblock, client_allowed_eblock, start, end;
  unsigned int block_size_x;
  struct horus_file_config c;
  char key[HORUS_MAX_KEY_LEN];
  size_t key_len = HORUS_MAX_KEY_LEN;

  fd = open (kreq->filename, O_RDONLY);
  if (fd < 0)
    {
      kresp->err = errno;
      return 0;
    }
  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      kresp->err = ret;
      return 0;
    }
  ret = horus_client_range_from_config (&c, client, &client_allowed_sblock,
                                        &client_allowed_eblock);
  if (ret < 0)
    {
      kresp->err = ret;
      return 0;
    }
  start = (kreq->y * c.kht_block_size[kreq->x]);
  end = ((kreq->y + 1) * c.kht_block_size[kreq->x]);
  if ((start >= client_allowed_sblock) && (end <= client_allowed_eblock))
    {
      horus_key_by_master (key, &key_len, kreq->x, kreq->y, c.master_key,
                           strlen (c.master_key), c.kht_block_size);
      memcpy (kresp->key, key, key_len);
      kresp->x = kreq->x;
      kresp->y = kreq->y;
      memcpy (&kresp->kht_block_size, &c.kht_block_size,
              sizeof (c.kht_block_size));
      kresp->err = 0;
    }
  else
    {
      kresp->err = EACCES;
    }
  return 0;
}

void *
handle_kds_client (void *p)
{
  int fd, th_val;
  int ret, client_len;
  struct sockaddr_in client;
  struct key_request_packet kreq;
  struct key_response_packet kresp;
  struct horus_clock clock_key, clock_total;

  if (benchmark)
    start_horus_clock (&clock_total);
  pthread_detach (pthread_self ());
  increment_thread_count ();
  fd = *(int *) p;
  client_len = sizeof (client);
  ret = recvfrom (fd, &kreq, sizeof (kreq), 0, (struct sockaddr *) &client,
                  &client_len);
  if (ret <= 0)
    {
      decrement_thread_count ();
      pthread_exit (NULL);
    }
  assert (ret == sizeof (key_request_packet));

/* Calculate and send key to client */

  if (benchmark)
    start_horus_clock (&clock_key);
  kds_get_client_key (&client.sin_addr, &kreq, &kresp);
  if (benchmark)
    stop_horus_clock (&clock_key);

  sendto (fd, &kresp, sizeof (kresp), 0, (struct sockaddr *) &client,
          client_len);
  decrement_thread_count ();
  if (benchmark)
    {
      stop_horus_clock (&clock_total);
      printf ("key ");
      print_horus_clock_time (&clock_key);
      printf (" total ");
      print_horus_clock_time (&clock_total);
      printf ("\n");
    }
  pthread_exit (NULL);
}

int
main (int argc, char **argv)
{
  int fd, ret, ch;
  fd_set fds;
  pthread_t th[HORUS_THREAD_MAX];

  progname = (1 ? "kds_server" : argv[0]);
  benchmark = 0;
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

  fd = server_socket (PF_INET, SOCK_DGRAM,
                      HORUS_KDS_SERVER_PORT, "kds_server_udp");
  assert (fd >= 0);
  FD_ZERO (&fds);
  FD_SET (fd, &fds);
  fcntl (fd, F_SETFL, O_NONBLOCK);
  printf ("KDS started!\n");
  while (1)
    {
      ret = select (fd + 1, &fds, NULL, NULL, NULL);
      assert (ret >= 0);
      if (thread_count () < HORUS_THREAD_MAX && FD_ISSET (fd, &fds))
        {
          pthread_create (&th[thread_count ()], NULL, handle_kds_client,
                          (void *) &fd);
        }
    }
  return 0;
}