
#include <horus.h>

#include <thread.h>
#include <network.h>

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

struct kdb_entry
{
  char *username;
  char *private_key;
};

struct thread_master *master = NULL;
struct vectorx *kdb = NULL;

DEFINE_COMMAND (show_thread_info,
                "show thread",
                "show\n"
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
      if (master->threads[i].state != THREAD_STATE_RELEASED)
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
  struct vectorx_node *n;
  struct kdb_entry *entry;

  for (n = vectorx_head (kdb); n; n = vectorx_next (n))
    {
      entry = (struct kdb_entry *) n->data;
      shell_printf (csh->shell, "user-key user %s private-key %s",
                    entry->username, entry->private_key);
    }
}

struct kdb_entry *
kdb_create (char *username, char *private_key)
{
  struct kdb_entry *entry;

  entry = (struct kdb_entry *) malloc (sizeof (struct kdb_entry));
  assert (entry);
  memset (entry, 0, sizeof (struct kdb_entry));

  entry->username = strdup (username);
  entry->private_key = strdup (private_key);

  return entry;
}

void
kdb_delete (struct kdb_entry *entry)
{
  free (entry->username);
  free (entry->private_key);
  free (entry);
}

int
kdb_compare (const void *a, const void *b)
{
  int ret;
  struct kdb_entry *ka = *(struct kdb_entry **) a;
  struct kdb_entry *kb = *(struct kdb_entry **) b;

  fprintf (stderr, "debug: ka: (%s, %s)\n", ka->username, ka->private_key);
  fprintf (stderr, "debug: kb: (%s, %s)\n", kb->username, kb->private_key);

  ret = strcmp (ka->username, kb->username);
  fprintf (stderr, "debug: return %d\n", ret);

  return ret;
}

DEFINE_COMMAND (user_key,
                "user-key user USERNAME private-key VALUE",
                "user's key information.\n"
                "specify username\n"
                "specify username\n"
                "specify private-key\n"
                "specify key value\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  struct kdb_entry *request, *exist;

  request = kdb_create (argv[2], argv[4]);
  exist = vectorx_lookup_bsearch (request, kdb_compare, kdb);
  if (exist)
    {
      free (exist->private_key);
      exist->private_key = strdup (argv[4]);
      kdb_delete (request);
      shell_printf (csh->shell, "user %s's private-key modified.",
                    exist->username);
    }
  else
    {
      vectorx_add (request, kdb);
      vectorx_sort (kdb_compare, kdb);
      shell_printf (csh->shell, "user %s's private-key added.",
                    request->username);
    }
}

DEFINE_COMMAND (no_user_key,
                "no user-key user USERNAME",
                "negate\n"
                "delete user's key information.\n"
                "specify username\n"
                "specify username\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  struct kdb_entry *request, *exist;

  request = kdb_create (argv[3], "dummy");
  exist = vectorx_lookup_bsearch (request, kdb_compare, kdb);
  if (exist)
    {
      vectorx_remove (exist, kdb);
      kdb_delete (exist);
      shell_printf (csh->shell, "user %s's private-key deleted.",
                    request->username);
    }
  kdb_delete (request);
}

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
  INSTALL_COMMAND (shell->cmdset, show_thread_info);
  INSTALL_COMMAND (shell->cmdset, show_user_key);
  INSTALL_COMMAND (shell->cmdset, user_key);
  INSTALL_COMMAND (shell->cmdset, no_user_key);

  shell_set_terminal (shell->shell, thread->readfd, thread->writefd);

  /* send telnet options. */
  write (shell->shell->writefd, will_echo, 3);
  write (shell->shell->writefd, will_suppress_go_ahead, 3);
  write (shell->shell->writefd, dont_linemode, 3);

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

  kdb = vectorx_create ();

  terminal_init ();

  while (thread_running (master))
    thread_run (master);

  vectorx_delete (kdb);

  thread_master_delete (master);

  return 0;
}


