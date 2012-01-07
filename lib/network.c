
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
server_socket (int domain, int type, u_int16_t port, char *service_name)
{
  int fd;
  int ret;
  struct sockaddr_in saddr;
  const int on = 1;
  char *dname, *tname;

  switch (domain)
    {
    case PF_INET:
      dname = "inet";
      break;
    default:
      dname = "other";
      break;
    }

  switch (type)
    {
    case SOCK_STREAM:
      tname = "tcp";
      break;
    case SOCK_DGRAM:
      tname = "udp";
      break;
    default:
      tname = "other";
      break;
    }

  fd = socket (domain, type, 0);
  if (fd < 0)
    {
      fprintf (stderr, "socket(%s, %s) failed: %s\n",
               dname, tname, strerror (errno));
      return -1;
    }

  ret = setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
  if (ret < 0)
    {
      fprintf (stderr, "setsockopt(SO_REUSEADDR) failed: %s\n",
               strerror (errno));
      /* continue. */
    }

  if (domain == PF_INET)
    {
      memset (&saddr, 0, sizeof (saddr));
      saddr.sin_family = AF_INET;
      saddr.sin_port = htons (port);
      saddr.sin_addr.s_addr = htonl (INADDR_ANY);

      ret = bind (fd, (struct sockaddr *) &saddr, sizeof (saddr));
      if (ret < 0)
        {
          fprintf (stderr, "bind() failed: %s\n", strerror (errno));
          close (fd);
          return -1;
        }
    }

  if (type == SOCK_STREAM)
    {
      ret = listen (fd, 5);
      if (ret < 0)
        {
          fprintf (stderr, "listen() failed: %s\n", strerror (errno));
          close (fd);
          return -1;
        }
    }

  fprintf (stderr, "listening for %s service on %s %s port %hu.\n",
           service_name, dname, tname, port);

  return fd;
}

