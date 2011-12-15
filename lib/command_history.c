void
command_history_delete (struct command_history *history)
{
  int i;
  for (i = 0; i < HISTORY_SIZE; i++)
    free (history->array[i]);
  free (history);
}

void
command_history_add (char *command_line,
                     struct command_history *history, struct shell *shell)
{
  if (! history)
    {
      history = malloc (sizeof (struct command_history));
      memset (history, 0, sizeof (struct command_history));
      shell->history = history;
    }

  if (history->array[history->last])
    history->current = HISTORY_NEXT (history->last);
  assert (! history->array[history->current]);
  history->array[history->current] = strdup (command_line);
  history->last = history->current;
  history->current = HISTORY_NEXT (history->current);
  if (history->array[history->current])
    free (history->array[history->current]);
  history->array[history->current] = NULL;
}

void
command_history_prev (struct shell *shell)
{
  int ceil, start, prev;
  struct command_history *history = shell->history;

  if (! history)
    return;

  ceil = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (ceil);

#if 0
  printf ("last: %d ceil: %d start: %d current: %d\n",
          history->last, ceil, start, history->current);
#endif

  /* the beginning of the history */
  if (history->current == start)
    return;

  prev = HISTORY_PREV (history->current);
  if (history->array[prev])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[prev]);
      history->current = prev;
    }
}

void
command_history_next (struct shell *shell)
{
  int floor, start, next;
  struct command_history *history = shell->history;

  if (! history)
    return;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

#if 0
  printf ("last: %d floor: %d start: %d current: %d\n",
          history->last, floor, start, history->current);
#endif

  /* wrapping */
  if (history->current == floor)
    return;

  next = HISTORY_NEXT (history->current);
  if (history->array[next])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[next]);
      history->current = next;
    }
}

DEFINE_COMMAND (show_history,
                "show history",
                "display information\n"
                "command history\n")
{
  struct shell *shell = (struct shell *) context;
  struct command_history *history = shell->history;
  int floor, start, i;

  if (! history)
    return;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

  for (i = start; i != floor; i = HISTORY_NEXT (i))
    if (history->array[i])
      fprintf (shell->terminal, "[%3d] %s\n", i, history->array[i]);
}


