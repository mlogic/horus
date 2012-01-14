
#include <arpa/telnet.h>

#include <log.h>
#include <shell.h>
#include <shell_history.h>
#include <command.h>
#include <command_shell.h>
#include <thread.h>
#include <terminal.h>

void (*terminal_install_commands) (struct command_set *cmdset) = NULL;

void *
terminal_service (void *arg)
{
  struct thread *thread = (struct thread *) arg;
  struct command_shell *shell;

  /* telnet options. */
  char will_echo[] = { IAC, WILL, TELOPT_ECHO, '\0' };
  char will_suppress_go_ahead[] = { IAC, WILL, TELOPT_SGA, '\0' };
  char dont_linemode[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };

  shell = command_shell_create ();
  shell_install (shell->shell, CONTROL('C'), NULL);
  shell_history_enable (shell->shell);
  INSTALL_COMMAND (shell->cmdset, show_history);

  if (terminal_install_commands)
    (*terminal_install_commands) (shell->cmdset);

  shell_set_terminal (shell->shell, thread->readfd, thread->writefd);

  /* send telnet options. */
  write (shell->shell->writefd, will_echo, 3);
  write (shell->shell->writefd, will_suppress_go_ahead, 3);
  write (shell->shell->writefd, dont_linemode, 3);

  command_shell_start (shell);

  while (command_shell_running (shell))
    command_shell_run (shell);

  shell_history_disable (shell->shell);

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

      t = thread_get (thread->master);
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
terminal_init (struct thread_master *master, int fd)
{
  struct thread *t;

  t = thread_get (master);
  assert (t);
  t->func = terminal_accept;
  t->name = "terminal_accept";
  t->readfd = fd;
  t->writefd = fd;
  t->arg = NULL;
  thread_add_read (t);
}

