#ifndef _HORUSIO_C_
#define _HORUSIO_C_

#define HORUSIO_ENCRYPT 1
#define HORUSIO_DECRYPT 2
ssize_t
horusio_crypt(char *buf,ssize_t size,ssize_t fdpos, int op);

int
horusio_open (const char *path, int oflag, ...);

ssize_t
horusio_read (int fd, void *buf, size_t size);

ssize_t
horusio_write (int fd, const void *buf, size_t size);

int
horusio_close (int fd);

int
hoursio_unlink (const char *path);

int
horusio_dup2 (int fd, int fd2);

#endif




