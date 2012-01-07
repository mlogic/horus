
#ifndef _SHELL_HISTORY_H_
#define _SHELL_HISTORY_H_

#include <vectorx.h>

struct shell_history
{
  struct vectorx *vector;
  int index;
};

void shell_history_add (char *line, struct shell_history *history);
void shell_history_prev (struct shell *shell, struct shell_history *history);
void shell_history_next (struct shell *shell, struct shell_history *history);

void shell_history_enable (struct shell *shell);
void shell_history_disable (struct shell *shell);

#endif /*_SHELL_HISTORY_H_*/

