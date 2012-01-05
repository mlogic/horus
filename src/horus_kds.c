
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

#define THREAD_STATE_NONE     0
#define THREAD_STATE_RELEASED 1
#define THREAD_STATE_ENDING   2
#define THREAD_STATE_MAX      3

struct thread {
  pthread_t pthread;
  int index;
  int state;
  void *(*func) (void *thread);
  int readfd;
  int writefd;
  void *arg;
  void *ret;
};

#define THREAD_MAX 256
struct thread threads[THREAD_MAX];

#define COND_THREAD 0
#define COND_MAX 1
pthread_mutex_t cond_mutex[COND_MAX];
pthread_cond_t cond[COND_MAX];

void
thread_init ()
{
  int i;

  memset (threads, 0, sizeof (threads));
  for (i = 0; i < THREAD_MAX; i++)
    {
      threads[i].index = i;
      threads[i].state = THREAD_STATE_RELEASED;
    }
}

struct thread *
thread_get ()
{
  int i;
  for (i = 0; i < THREAD_MAX; i++)
    {
      if (threads[i].state == THREAD_STATE_RELEASED)
        break;
    }

  if (i == THREAD_MAX)
    return NULL;

  return &threads[i];
}

void
thread_release (struct thread *t)
{
  t->state = THREAD_STATE_ENDING;
  pthread_cond_broadcast (&cond[COND_THREAD]);
}

int
thread_running (struct thread *master)
{
  return 1;
}

int
thread_run (struct thread *master)
{
  int i;

  pthread_mutex_lock (&cond_mutex[COND_THREAD]);
  pthread_cond_wait (&cond[COND_THREAD], &cond_mutex[COND_THREAD]);
  pthread_mutex_unlock (&cond_mutex[COND_THREAD]);

  for (i = 0; i < THREAD_MAX; i++)
    {
      if (threads[i].state == THREAD_STATE_ENDING)
        {
          pthread_join (threads[i].pthread, &threads[i].ret);
          threads[i].state = THREAD_STATE_RELEASED;
        }
    }

  return 0;
}

int
thread_add_read (struct thread *t)
{
  pthread_create (&t->pthread, NULL, t->func, t);
  return 0;
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

void *
terminal_service (void *arg)
{
  struct thread *thread = (struct thread *) arg;
  struct command_shell *shell;

  shell = command_shell_create ();
  shell_set_terminal (shell->shell, thread->readfd, thread->writefd);
  terminal_setting (shell);
  command_shell_start (shell);

  while (command_shell_running (shell))
    command_shell_run (shell);

  command_shell_delete (shell);

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

  acceptfd = thread->readfd;

  while (1)
    {
      fd = accept (acceptfd, (struct sockaddr *) &saddr, &saddr_len);
      if (fd < 0)
        {
          log_error ("accept() failed: %s\n", strerror (errno));
          return NULL;
        }

      t = thread_get ();
      assert (t);
      t->func = terminal_service;
      t->readfd = fd;
      t->writefd = fd;
      t->arg = NULL;
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

  fd = terminal_socket ();

  t = thread_get ();
  assert (t);
  t->func = terminal_accept;
  t->readfd = fd;
  t->writefd = fd;
  t->arg = NULL;
  thread_add_read (t);
}

int
main (int argc, char **argv)
{
  struct thread *master = NULL;

  thread_init ();
  //master = thread_master_create ();

  terminal_init ();

  while (thread_running (master))
    thread_run (master);

  /* Not reached */
  //thread_master_delete (master);

  return 0;
}


