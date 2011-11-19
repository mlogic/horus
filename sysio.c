#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>

ssize_t
read (int fd, void *buf, size_t nbyte)
{
  fprintf (stderr, "read() called with %d bytes.\n", (int) nbyte);
  return (ssize_t) syscall (SYS_read, fd, buf, nbyte);
}

ssize_t
write (int fd, const void *buf, size_t nbyte)
{
  fprintf (stderr, "read() called with %d bytes.\n", (int) nbyte);
  return (ssize_t) syscall (SYS_write, fd, buf, nbyte);
}

int
unlink (const char *path)
{
  fprintf (stderr, "Unlink %s.\n", path);
  return syscall (SYS_unlink, path);
}

