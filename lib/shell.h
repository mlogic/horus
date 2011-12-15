/*
 * Copyright (C) 2007  Yasuhiro Ohara
 */

#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define SHELL_WORD_DELIMITERS " "
#define SHELL_WORD_DELIMITERS_SUB " /"

#define CONTROL(X) ((X) - '@')

#ifndef FLAG_SET
#define FLAG_SET(V,F)        (V) = (V) | (F)
#define FLAG_CLEAR(V,F)      (V) = (V) & ~(F)
#define FLAG_CHECK(V,F)      ((V) & (F))
#endif /*FLAG_SET*/

struct shell;
typedef void (*shell_keyfunc_t) (struct shell *shell);

struct shell
{
  int flag;
  int readfd;
  int writefd;
  FILE *terminal;
  int interactive;

  char *prompt;
  char *command_line;
  int cursor;  /* cursor index, offset in command_line[] */
  int end;     /* end-of-string index, offset in command_line[] */
  int size;    /* buffer size of command_line[] */
  char *cut_buffer;

  shell_keyfunc_t key_func[128];

  void *context;
};

#define SHELL_FLAG_ESCAPE    0x01
#define SHELL_FLAG_EXIT      0x02
#define SHELL_FLAG_CLOSE     0x04
#define SHELL_FLAG_DEBUG     0x08

void shell_terminate (struct shell *shell);
void shell_format (struct shell *shell);
void shell_linefeed (struct shell *shell);
void shell_clear (struct shell *shell);
void shell_prompt (struct shell *shell);
void shell_refresh (struct shell *shell);

void shell_insert (struct shell *shell, char *s);
void shell_delete_string (struct shell *shell, int start, int end);
void shell_cut (struct shell *shell, int start, int end);
void shell_moveto (struct shell *shell, int index);
int shell_word_head (struct shell *shell, int point);
int shell_word_end (struct shell *shell, int point);

void shell_keyfunc_ctrl_a (struct shell *shell);
void shell_keyfunc_ctrl_b (struct shell *shell);
void shell_keyfunc_ctrl_d (struct shell *shell);
void shell_keyfunc_ctrl_e (struct shell *shell);
void shell_keyfunc_ctrl_f (struct shell *shell);
void shell_keyfunc_ctrl_h (struct shell *shell);
void shell_keyfunc_ctrl_i (struct shell *shell);
void shell_keyfunc_ctrl_j (struct shell *shell);
void shell_keyfunc_ctrl_k (struct shell *shell);
void shell_keyfunc_ctrl_m (struct shell *shell);
void shell_keyfunc_ctrl_r (struct shell *shell);
void shell_keyfunc_ctrl_u (struct shell *shell);
void shell_keyfunc_ctrl_w (struct shell *shell);
void shell_keyfunc_ctrl_y (struct shell *shell);
void shell_keyfunc_ctrl_lb (struct shell *shell);


void shell_close (struct shell *shell);
int shell_read (struct shell *shell);

struct shell *shell_create ();
void shell_delete (struct shell *shell);

void shell_set_terminal (struct shell *shell, int readfd, int writefd);
void shell_set_prompt (struct shell *shell, char *prompt);
void shell_install (struct shell *shell, unsigned char key,
                    shell_keyfunc_t func);

int shell_running (struct shell *shell);

#endif /*_SHELL_H_*/

