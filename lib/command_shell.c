/*
 * Copyright (C) 2007  Yasuhiro Ohara
 */

#include <command_shell.h>

DEFINE_COMMAND (exit,
                "exit",
                "exit\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  fprintf (csh->shell->terminal, "exit !\n");
  FLAG_SET (csh->shell->flag, SHELL_FLAG_EXIT);
  shell_close (csh->shell);
}

ALIAS_COMMAND (logout, exit, "logout", "logout\n");
ALIAS_COMMAND (quit, exit, "quit", "quit\n");

DEFINE_COMMAND (enable_shell_debugging,
                "enable shell debugging",
                "enable features\n"
                "enable shell settings\n"
                "enable shell debugging\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  fprintf (csh->shell->terminal, "enable shell debugging.\n");
  FLAG_SET (csh->shell->flag, SHELL_FLAG_DEBUG);
}

DEFINE_COMMAND (disable_shell_debugging,
                "disable shell debugging",
                "disable features\n"
                "disable shell settings\n"
                "disable shell debugging\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  fprintf (csh->shell->terminal, "disable shell debugging.\n");
  FLAG_CLEAR (csh->shell->flag, SHELL_FLAG_DEBUG);
}

void
command_shell_execute (struct shell *shell)
{
  struct command_shell *csh = (struct command_shell *) shell->context;
  int ret = 0;
  char *comment;

  /* break the line */
  shell_linefeed (shell);

  /* comment handling */
  comment = strpbrk (shell->command_line, "#!");
  if (comment)
    {
      shell->end = comment - shell->command_line;
      shell->cursor = (shell->cursor > shell->end ?
                       shell->end : shell->cursor);
      shell_terminate (shell);
    }

  shell_format (shell);

  if (! strlen (shell->command_line))
    {
      shell_clear (shell);
      shell_prompt (shell);
      return;
    }

  ret = command_execute (shell->command_line, csh->cmdset, csh);
  if (ret < 0)
    fprintf (shell->terminal, "no such command: %s\n", shell->command_line);
  //command_history_add (shell->command_line, shell->history, shell);

  /* FILE buffer must be flushed before raw-writing the same file */
  fflush (shell->terminal);

  shell_clear (shell);
  shell_prompt (shell);
}

void
command_shell_completion (struct shell *shell)
{
  struct command_shell *csh = (struct command_shell *) shell->context;
  char *completion = NULL;

  shell_moveto (shell, shell_word_end (shell, shell->cursor));
  completion = command_complete (shell->command_line, shell->cursor,
                                 csh->cmdset);
  if (completion)
    shell_insert (shell, completion);
}

void
command_shell_ls_filename (struct command_shell *csh, char *word)
{
  char *path;
  char *dirname;
  char *filename;
  int num = 0;
  DIR *dir;
  struct dirent *dirent;

  fprintf (csh->shell->terminal, "\n");
  path = file_path (word, &dirname, &filename);

  dir = opendir (dirname);
  if (dir == NULL)
    {
      free (path);
      return;
    }

  while ((dirent = readdir (dir)) != NULL)
    if (! strncmp (dirent->d_name, filename, strlen (filename)))
      {
        if (num % 4 == 0)
          fprintf (csh->shell->terminal, "  ");
        fprintf (csh->shell->terminal, "%-16s", dirent->d_name);
        if (num % 4 == 3)
          fprintf (csh->shell->terminal, "\n");
        num++;
      }
  fprintf (csh->shell->terminal, "\n");

  free (path);
}

void
command_shell_ls_candidate (struct shell *shell)
{
  struct command_shell *csh = (struct command_shell *) shell->context;
  int last_word_index;
  char *last_word;
  char *fixed_part;
  struct command_node *match;
  struct command_node *node;
  struct vectorx_node *vn;

  /* show-candidates only works for the end part. */
  if (shell->cursor != shell->end)
    {
      shell->cursor = shell->end;
      shell_linefeed (shell);
      shell_format (shell);
      shell_refresh (shell);
      return;
    }

  /* save fixed part and last word from the command line */
  last_word_index = shell_word_head (shell, shell->cursor);
  last_word = strdup (&shell->command_line[last_word_index]);
  fixed_part = strdup (shell->command_line);
  fixed_part[last_word_index] = '\0';

  /* begin a new line */
  shell_linefeed (shell);

  match = command_match_node (fixed_part, csh->cmdset);
  if (match)
    {
      /* if the fixed_part is executable, print <cr> with its helpstr */
      if (last_word_index == shell->cursor && match->func)
        fprintf (shell->terminal, "  %-16s %s\n",
                 "<cr>", match->helpstr);

      /* show candidates in current node */
      for (vn = vectorx_head (match->cmdvec); vn; vn = vectorx_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match (node->cmdstr, last_word))
            fprintf (shell->terminal, "  %-16s %s\n",
                     node->cmdstr, node->helpstr);

          /* additionally show filename candidates */
          if (file_spec (node->cmdstr))
            command_shell_ls_filename (csh, last_word);
        }
    }

  free (last_word);
  free (fixed_part);

  shell_format (shell);
  shell_refresh (shell);
}

void
command_shell_install_default (struct command_shell *csh)
{
  INSTALL_COMMAND (csh->cmdset, exit);
  INSTALL_COMMAND (csh->cmdset, quit);
  INSTALL_COMMAND (csh->cmdset, logout);
  INSTALL_COMMAND (csh->cmdset, enable_shell_debugging);
  INSTALL_COMMAND (csh->cmdset, disable_shell_debugging);
}

struct command_shell *
command_shell_create ()
{
  struct command_shell *csh;

  csh = (struct command_shell *) malloc (sizeof (struct command_shell));
  if (csh == NULL)
    return NULL;
  memset (csh, 0, sizeof (struct command_shell));

  csh->cmdset = command_set_create ();
  command_shell_install_default (csh);

  csh->shell = shell_create ();
  shell_install (csh->shell, CONTROL('J'), command_shell_execute);
  shell_install (csh->shell, CONTROL('M'), command_shell_execute);
  shell_install (csh->shell, CONTROL('I'), command_shell_completion);
  shell_install (csh->shell, '?', command_shell_ls_candidate);
  //shell_install (csh->shell, CONTROL('P'), command_history_prev);
  //shell_install (csh->shell, CONTROL('N'), command_history_next);
  csh->shell->context = csh;

  return csh;
}

void
command_shell_delete (struct command_shell *csh)
{
  command_set_delete (csh->cmdset);
  //command_history_delete (shell->history);
  shell_delete (csh->shell);
  free (csh);
}

void
command_shell_start (struct command_shell *csh)
{
  shell_prompt (csh->shell);
}

int
command_shell_running (struct command_shell *csh)
{
  return shell_running (csh->shell);
}

void
command_shell_run (struct command_shell *csh)
{
  shell_read (csh->shell);
}


