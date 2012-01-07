
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

void
shell_history_keyfunc_ctrl_j (struct shell *shell)
{
  if (shell->history)
    shell_history_add (shell->command_line, shell->history);
  command_shell_execute (shell);
}

DEFINE_COMMAND (show_history,
                "show history",
                "show\n"
                "show command-line history\n")
{
  struct command_shell *csh = (struct command_shell *) context;
  shell_history_show (csh->shell);
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
  shell_install (shell->shell, CONTROL('J'), shell_history_keyfunc_ctrl_j);
  INSTALL_COMMAND (shell->cmdset, show_history);

  command_shell_start (shell);

  while (command_shell_running (shell))
    command_shell_run (shell);

  command_shell_delete (shell);

  termio_finish ();

  return 0;
}


