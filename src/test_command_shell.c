
#include <command_shell.h>
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

int
main (int argc, char **argv)
{
  struct command_shell *shell;

  termio_init ();

  shell = command_shell_create ();
  shell_history_enable (shell->shell);
  shell_set_terminal (shell->shell, 0, 1);
  shell_install (shell->shell, CONTROL('C'), NULL);

  command_shell_start (shell);

  while (command_shell_running (shell))
    command_shell_run (shell);

  command_shell_delete (shell);

  termio_finish ();

  return 0;
}


