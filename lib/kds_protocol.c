#include <horus.h>
#include <stdio.h>
#include <sys/types.h>
#include <kds_protocol.h>
#include <assert.h>


char *horus_error_strings [] = {
  "horus: no error",
  "horus: error in opening file",
  "horus: error in getting config",
  "horus: error in setting config",
  "horus: no such client",
  "horus: request out of range",
  "horus: request not allowed",
  "horus: adjusted x",
  "horus: unknown error",
};

char *
horus_strerror (u_int16_t err)
{
  assert (err < HORUS_ERR_MAX);
  return horus_error_strings[err];
}

void
client_key_request (char *key, int *key_len, char *filename, int x, int y,
                   struct sockaddr_in *serv)
{
  int ret;
  struct key_request_packet req;
  struct key_response_packet res;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  int resx, resy, reskeylen;
  int reserr, ressuberr;

  if (server_fd < 0)
    server_fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (server_fd <= 0)
    fprintf(stderr, "error = %s\n", strerror(errno));
  assert (server_fd >= 0);

  memset (&req, 0, sizeof (req));
  req.x = htonl (x);
  req.y = htonl (y);
  snprintf (req.filename, sizeof (req.filename), "%s", filename);

  ret = sendto (server_fd, &req, sizeof (struct key_request_packet), 0,
                (struct sockaddr *) serv, sizeof (struct sockaddr_in));
  assert (ret == sizeof (struct key_request_packet));
  ret = recvfrom (server_fd, &res, sizeof (struct key_response_packet),  0,
                  (struct sockaddr *) &addr, &addrlen);
  assert (ret == sizeof (struct key_response_packet));

  resx = ntohl (res.x);
  resy = ntohl (res.y);
  if (x != resx || y != resy)
    {
      printf ("wrong key.\n");
      exit (1);
    }

  reserr = (int) ntohs (res.err);
  ressuberr = (int) ntohs (res.suberr);
  if (reserr || ressuberr)
    {
      printf ("err = %d: %s\n", reserr, horus_strerror (reserr));
      printf ("suberr = %d: %s\n", ressuberr, strerror (ressuberr));
      exit (1);
    }

  reskeylen = ntohl (res.key_len);
  memset (key, 0, *key_len);
  memcpy (key, res.key, reskeylen);
  *key_len = reskeylen;
}

void
horus_key_request (char *key, size_t *key_len, char *filename,
                   int x, int y, int sockfd, struct sockaddr_in *serv)
{
  int ret;
  struct key_request_packet req;
  struct key_response_packet res;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  int resx, resy, reskeylen;
  int reserr, ressuberr;

  memset (&req, 0, sizeof (req));
  req.x = htonl (x);
  req.y = htonl (y);
  snprintf (req.filename, sizeof (req.filename), "%s", filename);

  ret = sendto (sockfd, &req, sizeof (struct key_request_packet), 0,
                (struct sockaddr *) serv, sizeof (struct sockaddr_in));
  assert (ret == sizeof (struct key_request_packet));
  if (horus_verbose)
    printf ("requested key: K_%d,%d\n", x, y);

  ret = recvfrom (sockfd, &res, sizeof (struct key_response_packet),  0,
                  (struct sockaddr *) &addr, &addrlen);
  assert (ret == sizeof (struct key_response_packet));

  resx = ntohl (res.x);
  resy = ntohl (res.y);
  if (x != resx || y != resy)
    {
      printf ("wrong key: K_%d,%d\n", resx, resy);
      exit (1);
    }
  if (horus_verbose)
    printf ("received key: K_%d,%d\n", resx, resy);

  reserr = (int) ntohs (res.err);
  ressuberr = (int) ntohs (res.suberr);
  if (reserr || ressuberr)
    {
      printf ("err = %d: %s\n", reserr, horus_strerror (reserr));
      printf ("suberr = %d: %s\n", ressuberr, strerror (ressuberr));
      exit (1);
    }

  reskeylen = ntohl (res.key_len);
  memset (key, 0, *key_len);
  memcpy (key, res.key, reskeylen);
  *key_len = (size_t) reskeylen;
}


void
horus_key_request_spin (char *key, size_t *key_len, char *filename,
                        int x, int y, int sockfd, struct sockaddr_in *serv)
{
  int ret;
  struct key_request_packet req;
  struct key_response_packet res;
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  int resx, resy, reskeylen;
  int reserr, ressuberr;
  int read_count = 5, send_count = 3;
  int success = 0;

  memset (&req, 0, sizeof (req));
  req.x = htonl (x);
  req.y = htonl (y);
  snprintf (req.filename, sizeof (req.filename), "%s", filename);

  do {
      if (horus_verbose)
        printf ("request key: K_%d,%d\n", x, y);
      ret = sendto (sockfd, &req, sizeof (struct key_request_packet), 0,
                    (struct sockaddr *) serv, sizeof (struct sockaddr_in));
      assert (ret == sizeof (struct key_request_packet));
      //send_count--;

      do {
          usleep (1);
          ret = recvfrom (sockfd, &res, sizeof (struct key_response_packet),  0,
                          (struct sockaddr *) &addr, &addrlen);
          assert (ret == sizeof (struct key_response_packet));
          //read_count--;
    
          resx = ntohl (res.x);
          resy = ntohl (res.y);
          if (x != resx || y != resy)
            {
              printf ("wrong key: K_%d,%d\n", resx, resy);
              exit (1);
            }
          if (horus_verbose)
            printf ("received key: K_%d,%d\n", resx, resy);

          success++;
      } while (! success && read_count > 0);
  } while (! success && send_count > 0);

  reserr = (int) ntohs (res.err);
  ressuberr = (int) ntohs (res.suberr);
  if (reserr || ressuberr)
    {
      printf ("err = %d: %s\n", reserr, horus_strerror (reserr));
      printf ("suberr = %d: %s\n", ressuberr, strerror (ressuberr));
      exit (1);
    }

  reskeylen = ntohl (res.key_len);
  memset (key, 0, *key_len);
  memcpy (key, res.key, reskeylen);
  *key_len = (size_t) reskeylen;
}




