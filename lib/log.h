
#ifndef _LOG_H_
#define _LOG_H_

#include <unistd.h>
#include <sys/types.h>

#ifdef __linux__
#include <sys/types.h>
#endif

extern int log_on;

void log_init ();
void log_open (int fd, const char *path, int oflag, mode_t mode);
void log_socket (int fd, int domain, int type, int protocol);
void log_read (int fd, off_t fdpos, void *buf, size_t nbyte, size_t size);
void log_write (int fd, off_t fdpos, void *buf, size_t nbyte, size_t size);
void log_close (int fd);
void log_unlink (const char *path);
void log_dup2 (int fd, int fd2);

void log_mesg (int priority, char *message, ...);
void log_error (char *message, ...);

#endif /*_LOG_H_*/

