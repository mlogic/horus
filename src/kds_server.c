#include <horus.h>
#include <horus_attr.h>
#include <horus_key.h>
#include <network.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <benchmark.h>

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

const char *optstring = "bvdh";
const char *optusage = "\
-b, --benchmark   Turn on benchmarking\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-h, --help        Display this help and exit\n\
";
const struct option longopts[] = {
  { "benchmark",  no_argument,        NULL, 'b' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "help",       no_argument,        NULL, 'h' },
  { NULL,         0,                  NULL,  0  }
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
kds_get_client_key (struct in_addr *client,
                    key_request_packet *kreq,
                    key_response_packet *kresp)
{
  int fd, ret;
  unsigned int sblock, eblock, start, end;
  struct horus_file_config c;
  char key[HORUS_MAX_KEY_LEN];
  size_t key_len = HORUS_MAX_KEY_LEN;
  u_int32_t kreqx, kreqy;

  fd = open (kreq->filename, O_RDONLY);
  if (fd < 0)
    {
      kresp->err = htonl (errno);
      return 0;
    }

  ret = horus_get_file_config (fd, &c);
  close (fd);
  if (ret < 0)
    {
      kresp->err = htonl (ret);
      return 0;
    }

  ret = horus_get_client_range (&c, client, &sblock, &eblock);
  if (ret < 0)
    {
      kresp->err = htonl (ret);
      return 0;
    }

  kreqx = (u_int32_t) ntohl (kreq->x);
  kreqy = (u_int32_t) ntohl (kreq->y);

  if (kreqx >= HORUS_MAX_KHT_DEPTH)
    {
      kresp->err = htonl (EINVAL);
      return 0;
    }
  if (c.kht_block_size[kreqx] == 0)
    {
      kresp->err = htonl (EINVAL);
      return 0;
    }

  start = (kreqy * c.kht_block_size[kreqx]);
  end = ((kreqy + 1) * c.kht_block_size[kreqx]);
  if (! (sblock <= start && end <= eblock))
    {
      kresp->err = htonl (EACCES);
      return 0;
    }

  ret = horus_key_by_master (key, &key_len, kreqx, kreqy,
                             c.master_key, strlen (c.master_key),
                             c.kht_block_size);
  if (ret < 0)
    {
      kresp->err = htonl (EINVAL);
      return 0;
    }

  kresp->x = htonl (kreqx);
  kresp->y = htonl (kreqy);
  memcpy (kresp->filename, kreq->filename, sizeof (kresp->filename));
  kresp->err = htonl (0);
  memcpy (&kresp->kht_block_size, &c.kht_block_size,
          sizeof (c.kht_block_size));
  memcpy (kresp->key, key, key_len);
  kresp->key_len = htonl (key_len);

  return 0;
}

void *
handle_kds_client (void *p)
{
  int fd;
  int ret;
  socklen_t client_len;
  struct sockaddr_in client;
  struct key_request_packet kreq;
  struct key_response_packet kresp;
  struct horus_clock clock_key, clock_total;

  if (benchmark)
    start_horus_clock (&clock_total);

  pthread_detach (pthread_self ());
  increment_thread_count ();

  if (horus_verbose)
    printf ("thread[%d]: %s\n", thread_count (), __func__);

  fd = *(int *) p;
  client_len = (socklen_t) sizeof (client);
  ret = recvfrom (fd, &kreq, sizeof (kreq), 0, (struct sockaddr *) &client,
                  &client_len);
  if (ret <= 0)
    {
      if (horus_verbose)
        printf ("recvfrom() failed: %s\n", strerror (errno));

      decrement_thread_count ();
      pthread_exit (NULL);
    }
  assert (ret == sizeof (key_request_packet));

  if (benchmark)
    start_horus_clock (&clock_key);

  /* Calculate the key for the client */
  kds_get_client_key (&client.sin_addr, &kreq, &kresp);

  if (benchmark)
    stop_horus_clock (&clock_key);

  /* Send the key to the client */
  sendto (fd, &kresp, sizeof (kresp), 0, (struct sockaddr *) &client,
          client_len);

  if (benchmark)
    {
      stop_horus_clock (&clock_total);
      printf ("key ");
      print_horus_clock_time (&clock_key);
      printf (" total ");
      print_horus_clock_time (&clock_total);
      printf ("\n");
    }

  /* XXX decrement may better be done in the parent thread
     that is watchdogging. */
  decrement_thread_count ();
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
        case 'v':
          horus_verbose++;
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

  /* To support spurious return from Linux select() */
  fcntl (fd, F_SETFL, O_NONBLOCK);

  printf ("KDS started!\n");

  while (1)
    {
      FD_ZERO (&fds);
      FD_SET (fd, &fds);

      ret = select (fd + 1, &fds, NULL, NULL, NULL);
      assert (ret >= 0);

      if (FD_ISSET (fd, &fds))
        {
          /* XXX th[] might shift incorrectly. */
          if (thread_count () < HORUS_THREAD_MAX)
            pthread_create (&th[thread_count ()], NULL,
                            handle_kds_client, (void *) &fd);
        }
    }

  return 0;
}

