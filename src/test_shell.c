
#include <shell.h>

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
  t.c_lflag ^= ICANON | ECHO;
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
  struct shell *shell;

  termio_init ();

  shell = shell_create ();
  shell_set_terminal (shell, 0, 1);

  while (shell_running (shell))
    shell_read (shell);

  shell_delete (shell);

  termio_finish ();

  return 0;
}


