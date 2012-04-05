
#include <horus.h>

#include <log.h>
#include <shell.h>
#include <shell_history.h>
#include <command.h>
#include <command_shell.h>
#include <thread.h>
#include <network.h>
#include <terminal.h>
#include <openssl.h>

#include <horus_attr.h>
#include <horus_key.h>

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
  char *filename;
  EVP_PKEY *private_key;
};

struct thread_master *master = NULL;
struct vectorx *kdb = NULL;

DEFINE_COMMAND (show_thread,
                "show thread", "show\n" "show thread information.\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  int i;

  shell_printf (csh->shell,
                "state: NONE:%d, RELEASED: %d, RUNNING:%d, ENDING:%d",
                THREAD_STATE_NONE, THREAD_STATE_RELEASED,
                THREAD_STATE_RUNNING, THREAD_STATE_ENDING);
  shell_linefeed (csh->shell);
  for (i = 0; i < master->nthreads; i++)
    {
      if (master->threads[i].state == THREAD_STATE_RELEASED)
        continue;

      shell_printf (csh->shell, "thread[%d]: index: %d, state: %d, func: %s",
                    i, master->threads[i].index, master->threads[i].state,
                    master->threads[i].name);
      shell_linefeed (csh->shell);
    }
}

DEFINE_COMMAND (show_user_key,
                "show user-key", "show\n" "show user's key information.\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  struct vectorx_node *n;
  struct kdb_entry *entry;

  //int ret;

  for (n = vectorx_head (kdb); n; n = vectorx_next (n))
    {
      entry = (struct kdb_entry *) n->data;
      shell_printf (csh->shell, "user-key user %s private-key %s",
                    entry->username, entry->filename);
      shell_linefeed (csh->shell);

#if 0
      ret = PEM_write_PrivateKey (csh->shell->terminal,
                                  entry->private_key, NULL, NULL, 0, 0, NULL);
      if (ret == 0)
        shell_printf (csh->shell, "failed to display private key.");
      shell_linefeed (csh->shell);
#endif /*0 */
    }
}

struct kdb_entry *
kdb_create (char *username, char *filename)
{
  struct kdb_entry *entry;

  entry = (struct kdb_entry *) malloc (sizeof (struct kdb_entry));
  assert (entry);
  memset (entry, 0, sizeof (struct kdb_entry));

  entry->username = strdup (username);
  entry->filename = strdup (filename);
  entry->private_key = NULL;

  return entry;
}

void
kdb_delete (struct kdb_entry *entry)
{
  free (entry->username);
  free (entry->filename);
  if (entry->private_key)
    EVP_PKEY_free (entry->private_key);
  free (entry);
}

int
kdb_compare (const void *a, const void *b)
{
  int ret;
  struct kdb_entry *ka = *(struct kdb_entry **) a;
  struct kdb_entry *kb = *(struct kdb_entry **) b;

  fprintf (stderr, "debug: ka: (%s, %s)\n", ka->username, ka->filename);
  fprintf (stderr, "debug: kb: (%s, %s)\n", kb->username, kb->filename);

  ret = strcmp (ka->username, kb->username);
  fprintf (stderr, "debug: return %d\n", ret);

  return ret;
}

DEFINE_COMMAND (user_key,
                "user-key user USERNAME private-key PATH",
                "user's key information.\n"
                "specify username\n"
                "specify username\n"
                "specify private-key\n" "specify file path\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  struct kdb_entry *request, *exist;
  EVP_PKEY *pkey;

  pkey = openssl_load_private_key (argv[4], NULL);
  if (pkey == NULL)
    {
      shell_printf (csh->shell, "Cannot load private key file: %s", argv[4]);
      shell_linefeed (csh->shell);
      return;
    }

  request = kdb_create (argv[2], argv[4]);
  assert (request);

  exist = vectorx_lookup_bsearch (request, kdb_compare, kdb);
  if (exist)
    {
      /* replace filename */
      free (exist->filename);
      exist->filename = strdup (argv[4]);

      /* replace key */
      if (exist->private_key)
        EVP_PKEY_free (exist->private_key);
      exist->private_key = pkey;

      /* delete request */
      kdb_delete (request);

      shell_printf (csh->shell, "user %s's private-key modified.",
                    exist->username);
      shell_linefeed (csh->shell);
    }
  else
    {
      /* set private key */
      request->private_key = pkey;

      /* add to the kdb */
      vectorx_add (request, kdb);
      vectorx_sort (kdb_compare, kdb);

      shell_printf (csh->shell, "user %s's private-key added.",
                    request->username);
      shell_linefeed (csh->shell);
    }
}

DEFINE_COMMAND (no_user_key,
                "no user-key user USERNAME",
                "negate\n"
                "delete user's key information.\n"
                "specify username\n" "specify username\n")
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
      shell_linefeed (csh->shell);
    }
  kdb_delete (request);
}

void
horus_kds_install_commands (struct command_set *cmdset)
{
  INSTALL_COMMAND (cmdset, show_thread);
  INSTALL_COMMAND (cmdset, show_user_key);
  INSTALL_COMMAND (cmdset, user_key);
  INSTALL_COMMAND (cmdset, no_user_key);
}

int
main (int argc, char **argv)
{
  int fd;

#define THREAD_MAX 16
  master = thread_master_create (THREAD_MAX);

  kdb = vectorx_create ();

  fd = server_socket (PF_INET, SOCK_STREAM,
                      HORUS_KDS_TERMINAL_PORT, "terminal");

  terminal_init (master, fd);
  terminal_install_commands = horus_kds_install_commands;

  while (thread_running (master))
    thread_run (master);

  vectorx_delete (kdb);

  thread_master_delete (master);

  return 0;
}
