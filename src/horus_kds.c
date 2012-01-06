
#include <horus.h>

#include <thread.h>

#define HORUS_KEY_SERVER_PORT 6666
#define HORUS_KDS_TERMINAL_PORT 6667

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /*MAXPATHLEN*/

struct horus_key_range
{
  u_int32_t i;
  u_int32_t j;
};

struct horus_key
{
  struct horus_key_range range;
  u_int32_t flag;
#define HORUS_KEY_FLAG_ENCRYPTED 0x01
  u_int32_t len;
  char value[64];               /* variable length. */
};

struct file_attr
{
  u_int32_t bs;
  u_int32_t depth;
  struct horus_key master_key;
};

struct client_permission
{
  struct in_addr client;
  struct horus_key_range allowed_range;
};

/* C -> KDS */
struct horus_key_request
{
  u_int32_t path_len;
  char path[MAXPATHLEN];        /* variable length. */
  struct horus_key_range range; /* requested range. */
};

/* KDS -> MDS (by extended attribute) */
struct horus_file_attr_request
{
  u_int32_t path_len;
  char path[MAXPATHLEN];        /* variable length. */
  struct in_addr client;        /* client id. */
  struct horus_key_range range; /* requested range. */
};

/* MDS -> KDS (by extended attribute) */
struct horus_file_attr_response
{
  u_int32_t path_len;
  char path[MAXPATHLEN];        /* variable length. */
  u_int32_t bs;
  u_int32_t depth;
  struct horus_key master_key;  /* master key. (encrypted) */
  struct in_addr client;        /* client id. */
  struct horus_key_range range; /* allowed range. */
};

/* KDS -> C */
struct horus_key_response
{
  u_int32_t path_len;
  char path[MAXPATHLEN];        /* variable length. */
  u_int32_t bs;
  u_int32_t depth;
  struct horus_key_range range; /* allowed range. */
  struct horus_key key;
};

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

void
terminal_setting (struct command_shell *shell)
{
  char will_echo[] =              { IAC, WILL, TELOPT_ECHO, '\0' };
  char will_suppress_go_ahead[] = { IAC, WILL, TELOPT_SGA, '\0' };
  char dont_linemode[] =          { IAC, DONT, TELOPT_LINEMODE, '\0' };
  write (shell->shell->writefd, will_echo, 3);
  write (shell->shell->writefd, will_suppress_go_ahead, 3);
  write (shell->shell->writefd, dont_linemode, 3);
}

struct thread_master *master = NULL;

DEFINE_COMMAND (show_thread_info,
                "show thread information",
                "show\n"
                "show thread information.\n"
                "show thread information.\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  int i;

  shell_printf (csh->shell,
    "state: NONE:%d, RELEASED: %d, RUNNING:%d, ENDING:%d",
    THREAD_STATE_NONE, THREAD_STATE_RELEASED,
    THREAD_STATE_RUNNING, THREAD_STATE_ENDING);
  for (i = 0; i < master->nthreads; i++)
    {
      shell_printf (csh->shell, "thread[%d]: index: %d, state: %d, func: %s",
                    i, master->threads[i].index, master->threads[i].state,
                    master->threads[i].name);
    }
}

DEFINE_COMMAND (show_user_key,
                "show user-key",
                "show\n"
                "show user's key information.\n")
{
  struct command_shell *csh = (struct command_shell *) context;
}

DEFINE_COMMAND (set_user_key,
                "set user-key user USERNAME key VALUE",
                "set\n"
                "set user's key information.\n"
                "specify username\n"
                "specify username\n"
                "specify key value\n"
                "specify key value\n")
{
  struct command_shell *csh = (struct command_shell *) context;
}

void *
terminal_service (void *arg)
{
  struct thread *thread = (struct thread *) arg;
  struct command_shell *shell;

  shell = command_shell_create ();
  INSTALL_COMMAND (shell->cmdset, show_thread_info);
  INSTALL_COMMAND (shell->cmdset, show_user_key);
  INSTALL_COMMAND (shell->cmdset, set_user_key);

  shell_set_terminal (shell->shell, thread->readfd, thread->writefd);
  terminal_setting (shell);
  command_shell_start (shell);

  while (command_shell_running (shell))
    command_shell_run (shell);

  command_shell_delete (shell);

  fprintf (stderr, "thread[%d]: terminal service closed.\n", thread->index);
  thread_release (thread);

  return NULL;
}

void *
terminal_accept (void *arg)
{
  struct thread *thread = (struct thread *) arg;
  struct thread *t;

  int acceptfd, fd;
  struct sockaddr_in saddr;
  socklen_t saddr_len;
  char saddr_name[64];

  acceptfd = thread->readfd;

  while (1)
    {
      saddr_len = sizeof (struct sockaddr_in);
      fd = accept (acceptfd, (struct sockaddr *) &saddr, &saddr_len);
      if (fd < 0)
        {
          log_error ("accept() failed: %s\n", strerror (errno));
          return NULL;
        }

      inet_ntop (AF_INET, &saddr.sin_addr, saddr_name, sizeof (saddr_name));
      fprintf (stderr, "thread[%d]: accept connection from %s:%d.\n",
               thread->index, saddr_name, saddr.sin_port);

      t = thread_get (master);
      assert (t);
      t->func = terminal_service;
      t->name = "terminal_service";
      t->readfd = fd;
      t->writefd = fd;
      t->arg = NULL;

      fprintf (stderr, "thread[%d]: terminal service for %s:%d on fd %d.\n",
               t->index, saddr_name, saddr.sin_port, fd);

      thread_add_read (t);
    }

  thread_release (thread);
  return NULL;
}

void
terminal_init ()
{
  int fd;
  struct thread *t;

  fd = server_socket (PF_INET, SOCK_STREAM,
                      HORUS_KDS_TERMINAL_PORT, "terminal");

  t = thread_get (master);
  assert (t);
  t->func = terminal_accept;
  t->name = "terminal_accept";
  t->readfd = fd;
  t->writefd = fd;
  t->arg = NULL;
  thread_add_read (t);
}

int
main (int argc, char **argv)
{
#define THREAD_MAX 16
  master = thread_master_create (THREAD_MAX);

  terminal_init ();

  while (thread_running (master))
    thread_run (master);

  thread_master_delete (master);

  return 0;
}


