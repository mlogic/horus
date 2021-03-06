#include <horus.h>
#include <horus_key.h>
#include <kds_protocol.h>
#include <assert.h>
#include <getopt.h>
#include <horus_attr.h>
#include <horus_stats.h>
#include "timeval.h"

#define HORUS_BUG_ADDRESS "horus@soe.ucsc.edu"
char *progname;

struct thread_info *thread = NULL;

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

extern int horus_debug;
extern int horus_verbose;

int benchmark = 0;
int aggregate = 0;
int simulate = 0;
int spinwait = 0;
useconds_t useconds = 0;
long nanoseconds = 0;
int nsend = 3;
int nread = 10;

const char *optstring = "hvdbn:s:x:y:o:l:a:ewu:t:r:";
const char *optusage = "\
-h, --help        Display this help and exit\n\
-v, --verbose     Turn on verbose mode\n\
-d, --debug       Turn on debugging mode\n\
-b, --benchmark   Turn on benchmarking\n\
-n, --nthread     Specify the number of thread\n\
-s, --server      Specify server IP address A.B.C.D[:P] (default: %s:%d)\n\
-x, --key-x       Specify x to calculate key K_x,y\n\
-y, --key-y       Specify y to calculate key K_x,y\n\
-o, --offset      Specify file offset to access (in bytes, e.g., 1G)\n\
-l, --length      Specify size of the range to access (from offset, in bytes, e.g., 1G)\n\
-a, --aggregate   Specify the level of aggregation of request range keys\n\
-e, --simulation  Turn on simulation mode of leaf key calculation\n\
-w, --spinwait    Turn on spinning busy-wait mode\n\
-u, --usleep      Specify the microseconds to sleep before read in spinwait\n\
-t, --nanosleep   Specify the nanoseconds to sleep before read in spinwait\n\
-r, --reread      Specify the times to retry read\n\
";

const struct option longopts[] = {
  { "help",       no_argument,        NULL, 'h' },
  { "verbose",    no_argument,        NULL, 'v' },
  { "debug",      no_argument,        NULL, 'd' },
  { "benchmark",  no_argument,        NULL, 'b' },
  { "nthread",    required_argument,  NULL, 'n' },
  { "server",     required_argument,  NULL, 's' },
  { "key-x",      required_argument,  NULL, 'x' },
  { "key-y",      required_argument,  NULL, 'y' },
  { "offset",     required_argument,  NULL, 'o' },
  { "length",     required_argument,  NULL, 'l' },
  { "aggregate",  required_argument,  NULL, 'a' },
  { "simulation", required_argument,  NULL, 'e' },
  { "spinwait",   no_argument,        NULL, 'w' },
  { "usleep",     required_argument,  NULL, 'u' },
  { "nanosleep",  required_argument,  NULL, 't' },
  { "reread",     required_argument,  NULL, 'r' },
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
  unsigned long nblock;
  u_int32_t alevel;
  u_int32_t aboffset;
  unsigned long anblock;
  u_int32_t leaf_level;
  u_int32_t *kht_block_size;
  char *filename;
  char *server;
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
  int success;
  unsigned long send_count, read_count;

  id = info->id;

  memset (&info->stats, 0, sizeof (struct horus_stats));
  send_count = read_count = 0;

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      printf ("thread[%d]: Unable to open socket!: %s\n",
              id, strerror (errno));
      return NULL;
    }

  serv_addr = info->serv_addr;
  boffset = info->boffset;
  nblock = info->nblock;
  memset (&kreq, 0, sizeof (kreq));

  reqx = info->level;
  kreq.x = htonl (reqx);
  strncpy (kreq.filename, info->filename, sizeof (kreq.filename));

  if (benchmark || horus_verbose)
    printf ("thread[%d]: server %s:%d bsize %d boffset %lu nblock %lu\n",
            id, info->server, ntohs (serv_addr->sin_port),
            HORUS_BLOCK_SIZE, boffset, nblock);

  if (spinwait)
    {
      fcntl (fd, F_SETFL, O_NONBLOCK);
      printf ("thread[%d]: spinwait: usleep %d nanosleep %ld "
              "nsend %d nread %d\n",
              id, useconds, nanoseconds, nsend, nread);
    }

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < nblock; i++)
    {
      reqy = boffset + i;
      kreq.y = htonl (reqy);

      success = 0;
      send_count = nsend;
      do {
          ret = sendto (fd, &kreq, sizeof (key_request_packet), 0,
                        (struct sockaddr *) serv_addr,
                        sizeof (struct sockaddr_in));
          send_count--;
          if (ret != sizeof (key_request_packet))
            {
              if (horus_debug)
                printf ("thread[%d]: sendto(): failed: %d "
                        "send_count: %ld\n", id, ret, send_count);
              info->stats.sendfail++;
              continue;
            }
          else
            {
              if (horus_debug)
                printf ("thread[%d]: request %d,%d send_count: %ld\n",
                        id, reqx, reqy, send_count);
            }

          read_count = nread;
          do {
              if (spinwait)
                {
                  if (useconds)
                    usleep (useconds);
                  if (nanoseconds)
                    {
                      struct timespec nanospec;
                      nanospec.tv_sec = 0;
                      nanospec.tv_nsec = nanoseconds;
                      nanosleep (&nanospec, NULL);
                    }
                }

              addrlen = sizeof (struct sockaddr_in);
              ret = recvfrom (fd, &kresp, sizeof (key_response_packet), 0,
                              (struct sockaddr *) &addr, &addrlen);
              read_count--;
              if (ret != sizeof (key_response_packet))
                {
                  if (horus_debug)
                    printf ("thread[%d]: recvfrom(): failed: %d "
                            "read_count: %ld\n", id, ret, read_count);
                  info->stats.recvfail++;
                  continue;
                }
              else
                {
                  if (horus_debug)
                    printf ("thread[%d]: recvfrom(): received %d\n", id, ret);

                  resx = ntohl (kresp.x);
                  resy = ntohl (kresp.y);

                  if (resx == reqx && resy == reqy)
                    success++;
                  else
                    {
                      if (horus_debug)
                        printf ("thread[%d]: mismatch: "
                                "req: %d,%d: resp: %d,%d\n",
                                id, reqx, reqy, resx, resy);
                      info->stats.resmismatch++;
                    }
                }
          } while (! success && read_count > 0);
      } while (! success && send_count > 0);

      info->stats.sendretry += nsend - send_count - 1;
      info->stats.recvretry += nread - read_count - 1;

      if (! success)
        {
          if (horus_verbose)
            printf ("thread[%d]: give up K_%d,%d: resend: %lu reread: %lu\n",
                    id, reqx, reqy, send_count, read_count);
          info->stats.giveup++;
          continue;
        }
      info->stats.success++;

      kresperr = (int) ntohs (kresp.err);
      krespsuberr = (int) ntohs (kresp.suberr);
      krespkeylen = (int) ntohl (kresp.key_len);
      horus_stats_record (&info->stats, kresperr, krespsuberr);

      if (horus_verbose && ! benchmark)
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

      if (simulate && ! kresperr)
        {
          char key[HORUS_MAX_KEY_LEN];
          size_t key_len;
          int simx, simy;
          unsigned long sboffset, snblock;
          u_int32_t *kht_block_size;
          int j;

          assert (reqx == resx && reqy == resy);
          simx = info->leaf_level;
          kht_block_size = info->kht_block_size;

          sboffset = resy * (kht_block_size[resx] / kht_block_size[simx]);
          snblock = kht_block_size[resx];

          if (resx == info->leaf_level)
            {
              info->stats.keycalculated = info->stats.success;
            }
          else
            {
              for (j = 0; j < snblock; j++)
                {
                  simy = sboffset + j;
                  key_len = sizeof (key);
                  horus_block_key (key, &key_len, simx, simy,
                                   kresp.key, krespkeylen, resx, resy,
                                   kht_block_size);
                  info->stats.keycalculated++;
                  if (horus_verbose && ! benchmark)
                    printf ("thread[%d]: simulated: K_%d,%d = %s\n", id,
                            simx, simy, print_key (key, key_len));
                }
            }
        }
    }

  if (benchmark)
    gettimeofday (&end, NULL);

  close (fd);

  if (benchmark)
    {
      timeval_sub (&end, &start, &res);
      time = res.tv_sec + res.tv_usec * 0.000001;
      info->timeval = res;
      printf ("thread[%d]: %llu/%lu keys in %f secs ( %f q/s\n",
              id, info->stats.success, nblock, time, info->stats.success/time);
      if (simulate)
        printf ("thread[%d]: %llu keys calculated in %f secs ( %f q/s\n",
                id, info->stats.keycalculated, time,
                info->stats.keycalculated/time);
    }
  else if (horus_verbose)
    {
      printf ("thread[%d]: %llu/%lu keys processed.\n",
              id, info->stats.success, nblock);
      if (simulate)
        printf ("thread[%d]: %llu keys calculated\n",
                id, info->stats.keycalculated);
    }

  return NULL;
}

int
main (int argc, char **argv)
{
  int fd;
  int ret, ch, i;
  int keyspec, keyx, keyy;
  unsigned long long offset, length;
  unsigned long level, boffset, nblock;
  unsigned long alevel, aboffset, anblock;
  int branch;
  char *filename = NULL;
  char *endptr;
  unsigned long leaf_level = HORUS_DEFAULT_LEAF_LEVEL;
  struct horus_file_config c;
  int dumb_ass_mode = 0;
  struct horus_stats stats;
  struct timeval start, end, res;
  double time;

  u_int16_t port;
  char *portspec = NULL;
  char *server[HORUS_MAX_SERVER_NUM];
  struct sockaddr_in serv_addr[HORUS_MAX_SERVER_NUM];
  int nservers = 0;

  memset (&serv_addr, 0, sizeof (serv_addr));
  alevel = anblock = aboffset = 0;

  int nthread = 1;

  keyspec = 0;
  keyx = keyy = 0;
  offset = length = 0;
  level = boffset = nblock = 0;

  progname = (1 ? "kds_client" : argv[0]);
  while ((ch = getopt_long (argc, argv, optstring, longopts, NULL)) != -1)
    {
      switch (ch)
        {
        case 'v':
          horus_verbose++;
          break;
        case 'd':
          horus_debug++;
          break;
        case 'b':
          benchmark++;
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
          if (nservers >= HORUS_MAX_SERVER_NUM)
            break;
          server[nservers] = optarg;
          portspec = index (optarg, ':');
          if (portspec)
            *portspec++ = '\0';
          serv_addr[nservers].sin_family = AF_INET;
          ret = inet_pton (AF_INET, server[nservers],
                           &serv_addr[nservers].sin_addr);
          if (ret != 1)
            {
              fprintf (stderr, "invalid server address: %s\n", optarg);
              return -1;
            }
          if (portspec)
            {
              port = (u_int16_t) strtol (portspec, &endptr, 0);
              if (*endptr != '\0')
                {
                  fprintf (stderr, "invalid port number \'%c\' in %s\n",
                           *endptr, portspec);
                  return -1;
                }
              serv_addr[nservers].sin_port = htons (port);
            }
          else
            serv_addr[nservers].sin_port = htons (HORUS_KDS_SERVER_PORT);
          nservers++;
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
        case 'a':
          aggregate++;
          alevel = strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'e':
          simulate++;
          break;
        case 'w':
          spinwait++;
          break;
        case 'u':
          useconds = (useconds_t) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 't':
          nanoseconds = (long) strtol (optarg, &endptr, 0);
          if (*endptr != '\0')
            {
              fprintf (stderr, "invalid \'%c\' in %s\n", *endptr, optarg);
              return -1;
            }
          break;
        case 'r':
          nread = (int) strtol (optarg, &endptr, 0);
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

  if (nservers == 0)
    {
      /* default setting */
      server[nservers] = HORUS_KDS_SERVER_ADDR;
      serv_addr[nservers].sin_family = AF_INET;
      ret = inet_pton (AF_INET, server[nservers],
                       &serv_addr[nservers].sin_addr);
      assert (ret == 1);
      port = HORUS_KDS_SERVER_PORT;
      serv_addr[nservers].sin_port = htons (port);
      nservers++;
    }

  memset (&c, 0, sizeof (c));
  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      printf ("cannot open file: %s\n", filename);
      exit (1);
    }

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    {
      printf ("cannot read Horus config: %s\n", filename);
      exit (1);
    }
  close (fd);

  if (! horus_is_valid_config (&c))
    {
      printf ("invalid Horus config: %s\n", filename);
      exit (1);
    }

  /* calculate leaf level */
  for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
    if (c.kht_block_size[i])
      leaf_level = i;
  if (horus_verbose)
    printf ("leaf level: %lu\n", leaf_level);

  thread = (struct thread_info *)
    malloc (sizeof (struct thread_info) * nthread);
  assert (thread);
  memset (thread, 0, sizeof (struct thread_info) * nthread);

  /* start point */
  if (offset)
    {
      level = leaf_level;
      boffset = (offset ? offset / HORUS_BLOCK_SIZE : 0);
    }

  /* when key is specified (override the start point by offset) */
  if (keyspec)
    {
      level = keyx;
      boffset = keyy;
    }

  /* length */
  nblock = 1;
  if (length)
    {
      level = leaf_level;
      nblock = length / HORUS_BLOCK_SIZE;
    }

  /* when key is specified (override the length) */
  if (keyspec && simulate)
    {
      nblock = 1;
      for (level = keyx; level < leaf_level; level++)
        {
          branch = c.kht_block_size[level] / c.kht_block_size[level + 1];
          boffset *= branch;
          nblock *= branch;
        }
      if (horus_verbose)
        printf ("range by key spec: K_%d,%d leaf: K_%lu,%lu nblock: %lu\n",
                keyx, keyy, level, boffset, nblock);
    }

  /* aggregate */
  if (aggregate)
    {
      /* calculate range as the aggregate level */
      aboffset = boffset;
      anblock = nblock;
      for (i = level; alevel < i; i--)
        {
          branch = c.kht_block_size[i - 1] / c.kht_block_size[i];
          if (anblock / branch == 0)
            break;
          anblock = (anblock / branch) + (anblock % branch ? 1 : 0);
          aboffset /= branch;
        }
      alevel = i;
    }

  if (horus_verbose || benchmark)
    {
      printf ("nthread: %d\n", nthread);
      printf ("filename: %s\n", filename);
      if (benchmark)
        printf ("benchmark: ");
      if (dumb_ass_mode)
        printf ("dumb-ass: ");
      if (keyspec)
        printf ("keyx: %d keyy: %d\n", keyx, keyy);
      printf ("level: %lu boffset: %lu nblock: %lu\n",
              level, boffset, nblock);
      if (aggregate)
        printf ("alevel: %lu aboffset: %lu anblock: %lu\n",
                alevel, aboffset, anblock);
    }

  if (benchmark)
    gettimeofday (&start, NULL);

  for (i = 0; i < nthread; i++)
    {
      int unit;

      thread[i].id = i;
      thread[i].fd = fd;
      thread[i].filename = filename;
      thread[i].server = server[i % nservers];
      thread[i].serv_addr = &serv_addr[i % nservers];

      thread[i].leaf_level = leaf_level;
      //thread[i].kht_block_size = &c.kht_block_size[0];
      thread[i].kht_block_size = c.kht_block_size;

      if (aggregate)
        {
          thread[i].level = alevel;
          unit = anblock / nthread;
          if (i < anblock % nthread)
            {
              thread[i].boffset = aboffset + unit * i + i;
              thread[i].nblock = unit + 1;
            }
          else
            {
              thread[i].boffset = aboffset + unit * i + anblock % nthread;
              thread[i].nblock = unit;
            }
        }
      else
        {
          thread[i].level = level;
          unit = nblock / nthread;
          if (i < nblock % nthread)
            {
              thread[i].boffset = boffset + unit * i + i;
              thread[i].nblock = unit + 1;
            }
          else
            {
              thread[i].boffset = boffset + unit * i + nblock % nthread;
              thread[i].nblock = unit;
            }
        }

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
      printf ("benchmark: overall-requested: %llu keys in %f secs by %f q/s\n",
              stats.success, time, stats.success/time);
    }

  if (benchmark)
    {
      double qps = 0.0;
      unsigned long long total = 0;
      for (i = 0; i < nthread; i++)
        {
          if (thread[i].stats.success)
            {
              time = thread[i].timeval.tv_sec +
                     thread[i].timeval.tv_usec * 0.000001;
              total += thread[i].stats.success;
              qps += thread[i].stats.success / time;
            }
        }
      printf ("benchmark: per-thread-requested: %llu keys by %f q/s\n",
              total, qps);
    }

  if (benchmark && simulate)
    {
      double qps = 0.0;
      unsigned long long total = 0;
      for (i = 0; i < nthread; i++)
        {
          if (thread[i].stats.keycalculated)
            {
              time = thread[i].timeval.tv_sec +
                     thread[i].timeval.tv_usec * 0.000001;
              total += thread[i].stats.keycalculated;
              qps += thread[i].stats.keycalculated / time;
            }
        }
      printf ("benchmark: per-thread-simulated: %llu leaf keys by %f q/s\n",
              total, qps);
    }

  free (thread);

  return 0;
}


