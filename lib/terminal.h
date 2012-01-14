
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

extern void (*terminal_install_commands) (struct command_set *cmdset);

void *terminal_service (void *arg);
void *terminal_accept (void *arg);

void terminal_init (struct thread_master *master, int fd);

#endif /*_TERMINAL_H_*/

