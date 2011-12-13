/*
 * Copyright (C) 2007  Yasuhiro Ohara
 */

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <vectorx.h>

#define COMMAND_WORD_DELIMITERS " "
#define COMMAND_HELP_DELIMITERS "\n"

typedef void (*command_func_t) (void *context, int argc, char **argv);

struct command_node
{
  char *cmdstr;
  char *helpstr;
  command_func_t func;
  char *cmdmem;
  char *helpmem;

  struct vectorx *cmdvec;
};

struct command_set
{
  struct command_node *root;
};

struct command_header
{
  char *cmdstr;
  char *helpstr;
  command_func_t cmdfunc;
};

#define DEFINE_COMMAND(cmdname, cmdstr, helpstr)             \
  void cmdname ## _func (void *context, int argc, char **argv); \
  struct command_header cmdname ## _cmd =                    \
  {                                                          \
    cmdstr,                                                  \
    helpstr,                                                 \
    cmdname ## _func                                         \
  };                                                         \
  void cmdname ## _func (void *context, int argc, char **argv)

#define ALIAS_COMMAND(aliasname, cmdname, cmdstr, helpstr)   \
  struct command_header aliasname ## _cmd =                  \
  {                                                          \
    cmdstr,                                                  \
    helpstr,                                                 \
    cmdname ## _func                                         \
  };                                                         \

#define INSTALL_COMMAND(cmdset, cmdname)                     \
  command_install (cmdset, cmdname ## _cmd.cmdstr,           \
                   cmdname ## _cmd.helpstr, cmdname ## _cmd.cmdfunc)

#define EXTERN_COMMAND(cmdname) \
  extern struct command_header cmdname ## _cmd

struct command_set *command_set_create ();
void command_set_delete (struct command_set *cmdset);

#define SPEC_STR_IPV4         "A.B.C.D"
#define SPEC_STR_IPV4_PREFIX  "A.B.C.D/M"
#define SPEC_STR_RANGE        "<dddd-dddd>"
#define SPEC_STR_DOUBLE_RANGE "<d.ddd-d.ddd>"
#define SPEC_STR_DOUBLE       "<[-]ddd.ddd>"
#define SPEC_STR_DOUBLE2      "<[-]d.ddde[+-]dd>"
#define SPEC_STR_FILE         "<FILENAME>"
#define SPEC_STR_LINE         "LINE"

int is_command_match (char *spec, char *word);

void
command_install (struct command_set *cmdset,
                 char *command_line,
                 char *help_string,
                 command_func_t func);
int
command_execute (char *command_line, struct command_set *cmdset,
                 void *context);

char *
command_complete (char *command_line, int point,
                  struct command_set *cmdset);

struct command_node *
command_match_node (char *command_line, struct command_set *cmdset);

//int file_spec (char *spec);

#endif /*_COMMAND_H_*/


