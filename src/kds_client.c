#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <horus_attr.h>
#include <horus_stats.h>
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

const char *optstring = "bvdhn:s:p:x:y:o:l:r:w:";
const char *optusage = "\
-b, --benchmark   Turn on benchmarking\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-h, --help        Display this help and exit\n\
-n, --nthread     Specify the number of thread\n\
-s, --server      Specify server IP address (default: %s)\n\
-p, --port        Specify server UDP port (default: %d)\n\
-x, --key-x       Specify x to calculate key K_x,y\n\
-y, --key-y       Specify y to calculate key K_x,y\n\
-o, --offset      Specify file offset to access\n\
-l, --length      Specify size of the range to access (from offset)\n\
-r, --rthread     Specify the number of read (wait-response) thread\n\
-w, --wthread     Specify the number of write (request) thread\n\
";

const struct option longopts[] = {
  { "benchmark",  no_argument,        NULL, 'b' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "help",       no_argument,        NULL, 'h' },
  { "nthread",    required_argument,  NULL, 'n' },
  { "key-x",      required_argument,  NULL, 'x' },
  { "key-y",      required_argument,  NULL, 'y' },
  { "offset",     required_argument,  NULL, 'o' },
  { "length",     required_argument,  NULL, 'l' },
  { NULL,         0,                  NULL,  0  }
};

void
usage ()
{
  printf ("Usage : %s [OPTION...] <filename>\n", progname);
  printf ("\n");
  printf ("options are \n");
  printf (optusage, HORUS_KDS_SERVER_ADDR, HORUS_KDS_SERVER_PORT);
  printf ("Report bugs to %s\n", HORUS_BUG_ADDRESS);
  exit (0);
}

int
client_sendrecv (int fd, struct sockaddr_in *serv_addr,
                 const key_request_packet* kreq,
                 key_response_packet *kresp)
{
  int ret = -1;
  socklen_t addrlen;

  assert ((kreq != NULL) && (kresp != NULL));
  ret = sendto (fd, kreq, sizeof (key_request_packet), 0,
                (struct sockaddr *) serv_addr, sizeof (struct sockaddr_in));
  if (ret < 0)
    {
      printf ("sendto() failed: %s\n", strerror (errno));
      return -1;
    }
  if (ret != sizeof (key_request_packet))
    {
      printf ("sendto() ret: %d\n", ret);
    }

  do
    {
      addrlen = sizeof (struct sockaddr_in);
      ret = recvfrom (fd, kresp, sizeof (key_response_packet), 0,
                      (struct sockaddr *) serv_addr, &addrlen);
    }
  while (ret <= 0);
  assert (ret == sizeof (key_response_packet));
  return 0;
}

struct thread_arg {
  int id;
  int fd;
  u_int32_t level;
  u_int32_t boffset;
  int nblock;
  char *filename;
  struct sockaddr_in *serv_addr;
};

void *
thread_read_write (void *arg)
{
  struct thread_arg *targ = (struct thread_arg *) arg;
  int i;
  int id;
  int fd;
  struct timeval start, end, res;
  double time;
  struct horus_stats stats;
  unsigned long boffset, nblock;
  u_int32_t keyx, keyy;
  int kresperr, krespsuberr, krespkeylen;

  struct key_request_packet kreq;
  struct key_response_packet kresp;

  id = targ->id;
  fd = targ->fd;
  boffset = targ->boffset;
  nblock = targ->nblock;

  memset (&stats, 0, sizeof (stats));

  memset (&kreq, 0, sizeof (kreq));
  keyx = targ->level;
  kreq.x = htonl (keyx);
  strncpy (kreq.filename, targ->filename, sizeof (kreq.filename));

  if (benchmark || horus_verbose)
    printf ("thread[%d]: block size: %d boffset %lu nblock %lu\n",
            id, HORUS_BLOCK_SIZE, boffset, nblock);

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < nblock; i++)
    {
      keyy = boffset + i;
      kreq.y = htonl (keyy);
      client_sendrecv (fd, targ->serv_addr, &kreq, &kresp);

      kresperr = (int) ntohs (kresp.err);
      krespsuberr = (int) ntohs (kresp.suberr);
      krespkeylen = (int) ntohl (kresp.key_len);
      horus_stats_record (&stats, kresperr, krespsuberr);

      if (! benchmark)
        {
          printf ("thread[%d]: err = %d : %s\n", id,
                  kresperr, horus_strerror (kresperr));
          printf ("thread[%d]: suberr = %d : %s\n", id,
                  krespsuberr, strerror (krespsuberr));
          if (! kresperr)
            printf ("thread[%d]: key_%d,%d = %s\n", id, keyx, keyy,
                     print_key (kresp.key, krespkeylen));
        }
    }

  if (benchmark)
    gettimeofday (&end, NULL);

  //horus_stats_print (&stats);

  if (benchmark)
    {
      timeval_sub (&end, &start, &res);
      time = res.tv_sec + res.tv_usec * 0.000001;
      printf ("thread[%d]: %lu keys in %f secs ( %f q/s\n",
              id, nblock, time, nblock/time);
    }

  return NULL;
}

int
main (int argc, char **argv)
{
  int fd;
  int ret, ch, i;
  struct in_addr sin_addr;
  u_int16_t port;
  struct sockaddr_in serv_addr;
  int keyx, keyy;
  unsigned long long offset, length;
  unsigned long boffset, nblock;
  char *server = NULL, *filename = NULL;
  char *endptr;
  unsigned long level;
  unsigned long leaf_level = 0;
  struct horus_file_config c;
  int dumb_ass_mode = 0;

  int nthread = 0, rthread = 0, wthread = 0;
  pthread_t *thread;
  struct thread_arg *thread_arg;
  struct thread_ret *thread_ret;

  keyx = keyy = 0;
  offset = length = 0;

  /* default setting */
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
        case 'n':
          nthread = (int) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
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
          offset = canonical_byte_size (optarg, &endptr);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'l':
          length = canonical_byte_size (optarg, &endptr);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'r':
          rthread = (int) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'w':
          wthread = (int) strtol (optarg, &endptr, 0);
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

  if (rthread || wthread)
    {
      if (nthread)
        {
          if (rthread == 0)
            rthread = nthread - wthread;
          if (wthread == 0)
            wthread = nthread - rthread;
        }
      else
        {
          if (rthread == 0)
            rthread = 1;
          if (wthread == 0)
            wthread = 1;
        }
      nthread = rthread + wthread;
    }
  else if (nthread == 0)
    {
      nthread = 4;
    }

  assert (nthread > 1);
  if (nthread > HORUS_THREAD_MAX)
    {
      printf ("max #threads: %d\n", HORUS_THREAD_MAX);
      exit (1);
    }


  memset (&c, 0, sizeof (c));
  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      if (horus_verbose)
        printf ("cannot open file: without KHT config: %s\n", filename);
    }
  else
    {
      ret = horus_get_file_config (fd, &c);
      if (ret < 0)
        {
          if (horus_verbose)
            printf ("cannot read config: without KHT config: %s\n",
                    filename);
        }
      close (fd);
    }

  if (horus_is_valid_config (&c))
    {
      //block_size = HORUS_BLOCK_SIZE;
      for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
        if (c.kht_block_size[i])
          leaf_level = i;

      if (horus_verbose)
        printf ("leaf level: %lu\n", leaf_level);
    }

  memset (&serv_addr, 0, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons (port);
  serv_addr.sin_addr = sin_addr;

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "Unable to open socket!\n");
      exit (1);
    }

  thread = (pthread_t *) malloc (sizeof (pthread_t) * nthread);
  thread_arg = (struct thread_arg *)
    malloc (sizeof (struct thread_arg) * nthread);
  assert (thread && thread_arg);

  /* start point */
  if (keyx || keyy)
    {
      level = keyx;
      boffset = keyy;
    }
  else
    {
      level = leaf_level;
      boffset = (offset ? offset / HORUS_BLOCK_SIZE : 0);
    }

  /* length */
  nblock = 1;
  if (length)
    nblock = length / HORUS_BLOCK_SIZE;

  /* dumb-ass mode */
  if (dumb_ass_mode)
    {
      if (horus_is_valid_config (&c) && (keyx || keyy))
        nblock = c.kht_block_size[level];
      level = leaf_level;
    }

  if (horus_verbose)
    {
      printf ("nthread: %d\n", nthread);
      if (benchmark)
        printf ("benchmark: ");
      if (dumb_ass_mode)
        printf ("dumb-ass: ");
      printf ("level: %lu boffset: %lu nblock: %lu\n", level, boffset, nblock);
      printf ("keyx: %d keyy: %d\n", keyx, keyy);
    }

  for (i = 0; i < nthread; i++)
    {
      int unit;

      thread_arg[i].id = i;
      thread_arg[i].fd = fd;
      thread_arg[i].level = level;
      thread_arg[i].filename = filename;
      thread_arg[i].serv_addr = &serv_addr;

      unit = nblock / nthread;
      thread_arg[i].boffset = boffset + unit * i;
      thread_arg[i].nblock = unit;
      if (i + 1 == nthread)
        thread_arg[i].nblock += nblock % nthread;

#if 0
      if (rthread || wthread)
        {
          if (i < rthread)
            pthread_create (&thread[i], NULL,
                            thread_read, (void *) &thread_arg[i]);
          else
            pthread_create (&thread[i], NULL,
                            thread_write, (void *) &thread_arg[i]);
        }
      else
#endif
        pthread_create (&thread[i], NULL,
                        thread_read_write, (void *) &thread_arg[i]);
    }

  for (i = 0; i < nthread; i++)
    pthread_join (thread[i], NULL);

  free (thread);
  free (thread_arg);

  return 0;
}


