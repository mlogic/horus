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

const char *optstring = "bvdhn:";
const char *optusage = "\
-b, --benchmark   Turn on benchmarking\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-h, --help        Display this help and exit\n\
-n, --nthread     Specify the number of thread\n\
";
const struct option longopts[] = {
  { "benchmark",  no_argument,        NULL, 'b' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "help",       no_argument,        NULL, 'h' },
  { "nthread",    required_argument,  NULL, 'n' },
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

pthread_mutex_t kds_server_mutex = PTHREAD_MUTEX_INITIALIZER;

static int
kds_get_client_key (int id, struct in_addr *client,
                    key_request_packet *kreq,
                    key_response_packet *kresp)
{
  int fd, ret;
  unsigned int sblock, eblock, start, end;
  struct horus_file_config c;
  char key[HORUS_MAX_KEY_LEN];
  size_t key_len = HORUS_MAX_KEY_LEN;
  u_int32_t kreqx, kreqy;

  kreqx = (u_int32_t) ntohl (kreq->x);
  kreqy = (u_int32_t) ntohl (kreq->y);

  if (horus_verbose)
    printf ("thread[%d]: request: K_%d,%d file: %s\n",
            id, kreqx, kreqy, kreq->filename);

  fd = open (kreq->filename, O_RDONLY);
  if (fd < 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: open error\n", id);
      kresp->err = htons (HORUS_ERR_OPEN);
      kresp->suberr = htons (errno);
      return 0;
    }

  ret = horus_get_file_config (fd, &c);
  close (fd);
  if (ret < 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: get_file_config error\n", id);
      kresp->err = htons (HORUS_ERR_CONFIG_GET);
      kresp->suberr = htons (errno);
      return 0;
    }

  ret = horus_get_client_range (&c, client, &sblock, &eblock);
  if (ret < 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: no such client\n", id);
      kresp->err = htons (HORUS_ERR_NO_SUCH_CLIENT);
      kresp->suberr = htons (errno);
      return 0;
    }

  if (kreqx >= HORUS_MAX_KHT_DEPTH)
    {
      if (horus_verbose)
        printf ("thread[%d]: kreq x: %lu out of max_kht_depth: %d\n",
                id, (unsigned long) kreqx, HORUS_MAX_KHT_DEPTH);
      kresp->err = htons (HORUS_ERR_REQ_OUT_OF_RANGE);
      kresp->suberr = htons (EINVAL);
      return 0;
    }
#ifndef DISABLE_KEY_X_ADJUSTING
  if (c.kht_block_size[kreqx] == 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: adjusting x\n", id);
      do {
        kreqx--;
      } while (kreqx > 0 && c.kht_block_size[kreqx] == 0);
    }
#endif /*DISABLE_KEY_X_ADJUSTING*/
  if (c.kht_block_size[kreqx] == 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: kht_block_size[%d] == 0\n", id, kreqx);
      kresp->err = htons (HORUS_ERR_REQ_OUT_OF_RANGE);
      kresp->suberr = htons (EINVAL);
      return 0;
    }

  start = (kreqy * c.kht_block_size[kreqx]);
  end = ((kreqy + 1) * c.kht_block_size[kreqx]);
  if (! (sblock <= start && end <= eblock))
    {
      if (horus_verbose)
        printf ("thread[%d]: client not allowed\n", id);
      kresp->err = htons (HORUS_ERR_REQ_NOT_ALLOWED);
      kresp->suberr = htons (EINVAL);
      return 0;
    }

  ret = horus_key_by_master (key, &key_len, kreqx, kreqy,
                             c.master_key, strlen (c.master_key),
                             c.kht_block_size);
  if (ret < 0)
    {
      if (horus_verbose)
        printf ("thread[%d]: horus_key_by_master failed\n", id);
      kresp->err = htons (HORUS_ERR_UNKNOWN);
      kresp->suberr = htons (EINVAL);
      return 0;
    }

  /* send back the response */
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

struct thread_arg {
  int fd;
  int id;
};

void *
handle_kds_client (void *arg)
{
  struct thread_arg *targ = (struct thread_arg *) arg;
  int id;
  int fd;
  int ret;
  socklen_t client_len;
  struct sockaddr_in client;
  struct key_request_packet kreq;
  struct key_response_packet kresp;
  char buf[32];

  id = targ->id;
  fd = targ->fd;

  if (horus_verbose)
    printf ("thread[%d]: start\n", id);

  while (1)
    {
      client_len = (socklen_t) sizeof (client);
      memset (&kreq, 0, sizeof (kreq));
      memset (&kresp, 0, sizeof (kresp));

      pthread_mutex_lock (&kds_server_mutex);
      ret = recvfrom (fd, &kreq, sizeof (kreq), 0,
                      (struct sockaddr *) &client, &client_len);
      pthread_mutex_unlock (&kds_server_mutex);

      if (ret <= 0)
        {
          if (horus_verbose)
            printf ("thread[%d]: recvfrom() failed: %s\n",
                    id, strerror (errno));
          pthread_exit (NULL);
        }

      if (ret != sizeof (key_request_packet))
        {
          if (horus_verbose)
            printf ("thread[%d]: recvfrom(): invalid size: %d\n", id, ret);
          pthread_exit (NULL);
        }

      if (horus_verbose)
        {
          inet_ntop (AF_INET, &client.sin_addr, buf, sizeof (buf));
          printf ("thread[%d]: recvfrom(): %s\n", id, buf);
        }

      /* Calculate the key for the client */
      kds_get_client_key (id, &client.sin_addr, &kreq, &kresp);

      /* Send the key to the client */
      ret = sendto (fd, &kresp, sizeof (kresp), 0,
                    (struct sockaddr *) &client, client_len);

      if (horus_verbose)
        {
          printf ("thread[%d]: sendto(): %s: ret: %d\n", id, buf, ret);
        }
    }
}

int
main (int argc, char **argv)
{
  int fd, ch;
  char *endptr;
  int nthread = HORUS_THREAD_MAX;
  pthread_t *thread;
  struct thread_arg *thread_arg;
  int i;

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
        case 'n':
          nthread = (int) strtol (optarg, &endptr, 0);
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

  thread = (pthread_t *) malloc (sizeof (pthread_t) * nthread);
  assert (thread);
  thread_arg = (struct thread_arg *)
    malloc (sizeof (struct thread_arg) * nthread);
  assert (thread_arg);

  fd = server_socket (PF_INET, SOCK_DGRAM,
                      HORUS_KDS_SERVER_PORT, "kds_server_udp");
  assert (fd >= 0);

#if 0
  /* To support spurious return from Linux select() */
  fcntl (fd, F_SETFL, O_NONBLOCK);
#endif

  printf ("KDS started with %d threads !\n", nthread);

  for (i = 0; i < nthread; i++)
    {
      thread_arg[i].id = i;
      thread_arg[i].fd = fd;
      pthread_create (&thread[i], NULL,
                      handle_kds_client, (void *) &thread_arg[i]);
    }

  for (i = 0; i < nthread; i++)
    pthread_join (thread[i], NULL);

  free (thread);
  free (thread_arg);

  return 0;
}


