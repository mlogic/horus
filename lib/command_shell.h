/*
 * Copyright (C) 2007  Yasuhiro Ohara
 */

#ifndef _COMMAND_SHELL_H_
#define _COMMAND_SHELL_H_

#include <shell.h>
#include <command.h>

struct command_shell {
  struct shell *shell;
  struct command_set *cmdset;
};

EXTERN_COMMAND (exit);
EXTERN_COMMAND (quit);
EXTERN_COMMAND (logout);

EXTERN_COMMAND (enable_shell_debugging);
EXTERN_COMMAND (disable_shell_debugging);

struct command_shell *command_shell_create ();
void command_shell_delete (struct command_shell *csh);

#endif /*_COMMAND_SHELL_H_*/



