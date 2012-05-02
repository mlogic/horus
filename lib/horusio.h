#ifndef _HORUSIO_C_
#define _HORUSIO_C_
ssize_t
horusio_decrypt(char *buf, ssize_t size,ssize_t fdpos);

ssize_t
horusio_encrypt(char *buf, ssize_t size,ssize_t fdpos);
int
horusio_open (const char *path, int oflag, ...);

int
horusio_socket (int domain, int type, int protocol);

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




