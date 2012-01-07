
#include <stdlib.h>

#include <shell.h>
#include <shell_history.h>

struct shell_history *
shell_history_create ()
{
  struct shell_history *history;
  history = (struct shell_history *) malloc (sizeof (struct shell_history));
  if (! history)
    return NULL;

  history->index = 0;
  history->vector = vectorx_create ();
  if (! history->vector)
    {
      free (history);
      return NULL;
    }

  return history;
}

void
shell_history_delete (struct shell_history *history)
{
  struct vectorx_node *n;
  for (n = vectorx_head (history->vector); n; n = vectorx_next (n))
    {
      if (n->data)
        free (n->data);
    }

  vectorx_delete (history->vector);
  free (history);
}

void
shell_history_add (char *line, struct shell_history *history)
{
  char *dup;

  if (! strlen (line))
    return;

  if (history->vector->size >= 1024)
    return;

  dup = strdup (line);
  vectorx_add (dup, history->vector);
  history->index = history->vector->size;
}

void
shell_bell (struct shell *shell)
{
  write (shell->writefd, "\a", 1);
}

void
shell_history_prev (struct shell *shell, struct shell_history *history)
{
  if (history->index <= 0)
    {
      shell_bell (shell);
      return;
    }

  history->index--;

  /* delete all */
  shell_delete_string (shell, 0, shell->end);

  /* insert history */
  if (history->index < history->vector->size)
    shell_insert (shell, vectorx_get (history->vector, history->index));
}

void
shell_history_next (struct shell *shell, struct shell_history *history)
{
  if (history->index >= history->vector->size)
    {
      shell_bell (shell);
      return;
    }

  history->index++;

  /* delete all */
  shell_delete_string (shell, 0, shell->end);

  /* insert history */
  if (history->index < history->vector->size)
    shell_insert (shell, vectorx_get (history->vector, history->index));
}

void
shell_history_keyfunc_ctrl_p (struct shell *shell)
{
  if (shell->history)
    shell_history_prev (shell, shell->history);
}

void
shell_history_keyfunc_ctrl_n (struct shell *shell)
{
  if (shell->history)
    shell_history_next (shell, shell->history);
}

void
shell_history_enable (struct shell *shell)
{
  shell->history = shell_history_create ();
  shell_install (shell, CONTROL('P'), shell_history_keyfunc_ctrl_p);
  shell_install (shell, CONTROL('N'), shell_history_keyfunc_ctrl_n);
}

void
shell_history_disable (struct shell *shell)
{
  if (! shell->history)
    return;
  shell_install (shell, CONTROL('P'), NULL);
  shell_install (shell, CONTROL('N'), NULL);
  shell_history_delete (shell->history);
  shell->history = NULL;
}

void
shell_history_show (struct shell *shell)
{
  struct vectorx_node *n;
  struct shell_history *history;

  history = (struct shell_history *) shell->history;
  if (! history)
    return;

  for (n = vectorx_head (history->vector); n; n = vectorx_next (n))
    {
      char *line = (char *) n->data;
      shell_printf (shell, "  %s", line);
    }
}


