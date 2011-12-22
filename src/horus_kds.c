
#include <horus.h>

#define HORUS_KEY_SERVER_PORT 6666
#define HORUS_KDS_TERMINAL_PORT 6667

char *progname;

int
acceptsock ()
{
  int fd;
  int ret;
  struct sockaddr_in saddr;

  fd = socket (PF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "socket() failed: %s\n", strerror (errno));
      return -1;
    }

  memset (&saddr, 0, sizeof (saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons (HORUS_KDS_TERMINAL_PORT);
  saddr.sin_addr.s_addr = htonl (INADDR_ANY);

  ret = bind (fd, (struct sockaddr *) &saddr, sizeof (saddr));
  if (ret < 0)
    {
      fprintf (stderr, "bind() failed: %s\n", strerror (errno));
      close (fd);
      return -1;
    }

  ret = listen (fd, 5);
  if (ret < 0)
    {
      fprintf (stderr, "listen() failed: %s\n", strerror (errno));
      close (fd);
      return -1;
    }

  return fd;
}

int
keyservsock ()
{
  int fd;
  int ret;
  struct sockaddr_in saddr;

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "socket() failed: %s\n", strerror (errno));
      return -1;
    }

  memset (&saddr, 0, sizeof (saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons (HORUS_KEY_SERVER_PORT);
  saddr.sin_addr.s_addr = htonl (INADDR_ANY);

  ret = bind (fd, (struct sockaddr *) &saddr, sizeof (saddr));
  if (ret < 0)
    {
      fprintf (stderr, "bind() failed: %s\n", strerror (errno));
      close (fd);
      return -1;
    }

  return fd;
}

int
main (int argc, char **argv)
{
  struct sockaddr_in caddr;
  socklen_t caddr_len = sizeof (caddr);

  int ret;
  char buf[4098];
  int keyservfd, acceptfd;
  fd_set readfds;

  keyservfd = keyservsock ();
  acceptfd = acceptsock ();

  FD_ZERO (&readfds);
  FD_SET (keyservfd, &readfds);
  FD_SET (acceptfd, &readfds);

  while (1)
    {
      ret = recvfrom (keyservfd, buf, sizeof (buf), 0,
                      (struct sockaddr *) &caddr, &caddr_len);
      if (ret < 0)
        {
          fprintf (stderr, "recvfrom() failed: %s\n", strerror (errno));
          close (keyservfd);
          exit (-1);
        }

      fprintf (stdout, "recvfrom %s:%d\n",
               inet_ntoa (caddr.sin_addr), ntohs (caddr.sin_port));
    }

  return 0;
}


