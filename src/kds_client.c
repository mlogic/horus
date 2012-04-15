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

struct thread_info *thread = NULL;

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern int horus_debug;
extern int horus_verbose;

const char *optstring = "bvdhn:s:p:x:y:o:l:";
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

struct thread_info {
  pthread_t pthread;
  int id;
  int fd;
  u_int32_t level;
  u_int32_t boffset;
  int nblock;
  char *filename;
  struct sockaddr_in *serv_addr;
  struct timeval timeval;
  struct horus_stats stats;
};

void *
thread_read_write (void *arg)
{
  struct thread_info *info = (struct thread_info *) arg;
  int i, ret;
  int id, fd;

  /* for request */
  u_int32_t reqx, reqy;
  unsigned long boffset, nblock;
  struct sockaddr_in *serv_addr;
  struct key_request_packet kreq;

  /* for read */
  u_int32_t resx, resy;
  struct sockaddr_in addr;
  socklen_t addrlen;
  struct key_response_packet kresp;
  int kresperr, krespsuberr, krespkeylen;

  struct timeval start, end, res;
  double time;
  int success, resend, reread;
  const int nresend = 2, nreread = 2;

  id = info->id;
  fd = info->fd;

  memset (&info->stats, 0, sizeof (struct horus_stats));

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      printf ("thread[%d]: Unable to open socket!: %s\n",
              id, strerror (errno));
      return NULL;
    }
  //fcntl (fd, F_SETFL, O_NONBLOCK);

  serv_addr = info->serv_addr;
  boffset = info->boffset;
  nblock = info->nblock;
  memset (&kreq, 0, sizeof (kreq));

  reqx = info->level;
  kreq.x = htonl (reqx);
  strncpy (kreq.filename, info->filename, sizeof (kreq.filename));

  if (benchmark || horus_verbose)
    printf ("thread[%d]: block size: %d boffset %lu nblock %lu\n",
            id, HORUS_BLOCK_SIZE, boffset, nblock);

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < nblock; i++)
    {
      reqy = boffset + i;
      kreq.y = htonl (reqy);

      success = 0;
      resend = nresend;
      do {
          ret = sendto (fd, &kreq, sizeof (key_request_packet), 0,
                        (struct sockaddr *) serv_addr,
                        sizeof (struct sockaddr_in));
          if (ret != sizeof (key_request_packet))
            {
              if (horus_verbose)
                printf ("thread[%d]: sendto(): failed: %d, skip\n", id, ret);
              info->stats.sendfail++;
              continue;
            }

          reread = nreread;
          do {
              addrlen = sizeof (struct sockaddr_in);
              ret = recvfrom (fd, &kresp, sizeof (key_response_packet), 0,
                              (struct sockaddr *) &addr, &addrlen);
              printf ("thread[%d]: recvfrom(): %d\n", id, ret);
              if (ret != sizeof (key_response_packet))
                {
                  if (horus_verbose)
                    printf ("thread[%d]: recvfrom(): failed: %d\n", id, ret);
                  info->stats.recvfail++;
                  continue;
                }
              else
                success++;
          } while (! success && reread--);
      } while (! success && resend--);

      info->stats.sendretry += nresend - resend;
      info->stats.recvretry += nreread - reread;

      if (! success)
        {
          info->stats.giveup++;
          continue;
        }
      info->stats.success++;

      kresperr = (int) ntohs (kresp.err);
      krespsuberr = (int) ntohs (kresp.suberr);
      krespkeylen = (int) ntohl (kresp.key_len);
      horus_stats_record (&info->stats, kresperr, krespsuberr);

      resx = ntohl (kresp.x);
      resy = ntohl (kresp.y);

      if (resx != reqx || resy != reqy)
        info->stats.resmismatch++;

      if (! benchmark)
        {
          if (kresperr)
            printf ("thread[%d]: err = %d : %s\n", id,
                    kresperr, horus_strerror (kresperr));
          if (krespsuberr)
            printf ("thread[%d]: suberr = %d : %s\n", id,
                    krespsuberr, strerror (krespsuberr));
          if (! kresperr)
            printf ("thread[%d]: key_%d,%d: key_%d,%d/%d = %s\n", id,
                    reqx, reqy, resx, resy, krespkeylen,
                    print_key (kresp.key, krespkeylen));
        }
    }

  close (fd);

  if (! benchmark && horus_verbose)
    printf ("thread[%d]: %llu keys processed.\n",
            id, info->stats.success);

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
  int keyspec, keyx, keyy;
  unsigned long long offset, length;
  unsigned long boffset, nblock;
  char *server = NULL, *filename = NULL;
  char *endptr;
  unsigned long level;
  unsigned long leaf_level = 0;
  struct horus_file_config c;
  int dumb_ass_mode = 0;
  struct horus_stats stats;
  struct timeval start, end, res;
  double time;

  int nthread = 1;

  keyspec = 0;
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
          keyspec++;
          break;
        case 'y':
          keyy = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          keyspec++;
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

  if (nthread < 1 || HORUS_THREAD_MAX < nthread)
    {
      printf ("#threads must be: 1 - %d\n", HORUS_THREAD_MAX);
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

  thread = (struct thread_info *)
    malloc (sizeof (struct thread_info) * nthread);
  assert (thread);
  memset (thread, 0, sizeof (struct thread_info) * nthread);

  /* start point */
  if (keyspec)
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
      if (keyspec && horus_is_valid_config (&c))
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

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < nthread; i++)
    {
      int unit;

      thread[i].id = i;
      thread[i].fd = fd;
      thread[i].level = level;
      thread[i].filename = filename;
      thread[i].serv_addr = &serv_addr;

      unit = nblock / nthread;
      thread[i].boffset = boffset + unit * i;
      thread[i].nblock = unit;
      if (i + 1 == nthread)
        thread[i].nblock += nblock % nthread;

      pthread_create (&thread[i].pthread, NULL,
                      thread_read_write, (void *) &thread[i]);
    }

  for (i = 0; i < nthread; i++)
    pthread_join (thread[i].pthread, NULL);

  if (benchmark)
    gettimeofday (&end, NULL);

  memset (&stats, 0, sizeof (stats));
  for (i = 0; i < nthread; i++)
    horus_stats_merge (&stats, &thread[i].stats);
  horus_stats_print (&stats);

  if (benchmark)
    {
      timeval_sub (&end, &start, &res);
      time = res.tv_sec + res.tv_usec * 0.000001;
      printf ("horus benchmark: %llu keys in %f secs ( %f q/s\n",
              stats.success, time, stats.success/time);
    }

  free (thread);

  return 0;
}


