
#include <horus.h>

#define HORUS_KEY_SERVER_PORT 6666

char *progname;

int
main (int argc, char **argv)
{
  int fd;
  int ret;
  struct sockaddr_in saddr;
  struct sockaddr_in caddr;
  socklen_t caddr_len = sizeof (caddr);
  char buf[4098];

  fd = socket (PF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    {
      fprintf (stderr, "socket() failed: %s\n", strerror (errno));
      exit (-1);
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
      exit (-1);
    }

  while (1)
    {
      ret = recvfrom (fd, buf, sizeof (buf), 0,
                      (struct sockaddr *) &caddr, &caddr_len);
      if (ret < 0)
        {
          fprintf (stderr, "recvfrom() failed: %s\n", strerror (errno));
          close (fd);
          exit (-1);
        }

      fprintf (stdout, "recvfrom %s:%d\n",
               inet_ntoa (caddr.sin_addr), ntohs (caddr.sin_port));
    }
  return 0;
}


