
#include <shell.h>
#include <shell_history.h>

#include <termios.h>

struct termios oterm;

void
termio_init ()
{
  struct termios t;

  /* save original terminal settings */
  tcgetattr (0, &oterm);

  /* disable canonical input */
  memcpy (&t, &oterm, sizeof (t));
  t.c_lflag ^= ICANON | ECHO | ISIG;
  //t.c_oflag |= ONOCR;

  /* change terminal settings */
  tcsetattr (0, TCSANOW, &t);
}

void
termio_finish ()
{
  /* restore terminal settings */
  tcsetattr (0, TCSANOW, &oterm);
}

void
shell_history_keyfunc_ctrl_j (struct shell *shell)
{
  shell_linefeed (shell);

  shell_terminate (shell);
  shell_format (shell);

  if (shell->history)
    shell_history_add (shell->command_line, shell->history);

  shell_clear (shell);
  shell_prompt (shell);
}

int
main (int argc, char **argv)
{
  struct shell *shell;

  termio_init ();

  shell = shell_create ();
  shell_history_enable (shell);
  shell_set_terminal (shell, 0, 1);
  shell_install (shell, CONTROL ('J'), shell_history_keyfunc_ctrl_j);

  shell_start (shell);

  while (shell_running (shell))
    shell_read (shell);

  shell_history_disable (shell);
  shell_delete (shell);

  termio_finish ();

  return 0;
}
