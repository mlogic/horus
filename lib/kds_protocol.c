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
  int fd;
  int reserr, ressuberr;

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd <= 0)
    fprintf(stderr, "error = %s\n", strerror(errno));
  assert (fd >= 0);

  memset (&req, 0, sizeof (req));
  req.x = htonl (x);
  req.y = htonl (y);
  snprintf (req.filename, sizeof (req.filename), "%s", filename);

  ret = sendto (fd, &req, sizeof (struct key_request_packet), 0,
                (struct sockaddr *) serv, sizeof (struct sockaddr_in));
  assert (ret == sizeof (struct key_request_packet));
  ret = recvfrom (fd, &res, sizeof (struct key_response_packet),  0,
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
  close(fd);
}

