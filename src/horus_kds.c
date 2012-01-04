
#include <horus.h>

#define HORUS_KEY_SERVER_PORT 6666
#define HORUS_KDS_TERMINAL_PORT 6667

char *progname;

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
terminal_socket ()
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

#if 0

void *
terminal_accept (void *thread_arg)
{
  struct thread_arg *arg;
  int acceptfd, fd;
  struct sockaddr_in saddr;
  socklen_t saddr_len;

  arg = (struct thread_arg *) thread_arg;
  acceptfd = thread_arg_get_fd (arg);
  thread_arg_free (arg);

  fd = accept (acceptfd, (struct sockaddr *) &saddr, &saddr_len);
  if (fd < 0)
    {
      fprintf (stderr, "accept() failed: %s\n", strerror (errno));
      return NULL;
    }

  shell = shell_create ();
  shell_set_terminal (shell, fd, fd);

  arg = thread_arg_get ();
  thread_arg_set_val (arg, shell);
  thread_add_read (master, terminal_read, arg);

  return NULL;
}

void *
terminal_read (void *thread_arg)
{
  struct thread_arg *arg;
  struct shell *shell;

  arg = (strcut thread_arg *) thread_arg;
  shell = (struct shell *) thread_arg_get_val (arg);
  thread_arg_free (arg);

  while (shell_running (shell))
    shell_read (shell);

  return NULL;
}

void
terminal_init ()
{
  struct thread_arg *arg;
  int fd;
  fd = terminal_socket ();
  if (fd < 0)
    exit (-1);

  arg = thread_arg_get ();
  thread_arg_set_fd (arg, fd);
  thread_add_read (master, terminal_accept, arg);
}

void
key_server_init ()
{
}

int
main (int argc, char **argv)
{
  struct thread_master *master;

  master = thread_master_create ();

  terminal_init ();
  key_server_init ();

  while (thread_running (master))
    thread_run (master);

  /* Not reached */
  thread_master_delete (master);

  return 0;
}

#endif

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

  acceptfd = terminal_socket ();
  if (acceptfd < 0)
    exit (-1);

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


