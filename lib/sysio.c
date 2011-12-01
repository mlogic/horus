
#include <horus.h>

int
open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int fd = 0;

  if (oflag & O_CREAT)
    {
      va_list ap;
      va_start (ap, oflag);
      mode = va_arg (ap, int);
      va_end (ap);
      fd = (int) syscall (SYS_open, path, oflag, mode);
    }
  else
    {
      fd = (int) syscall (SYS_open, path, oflag);
    }

  log_open (fd, path, oflag, mode);

  return fd;
}

ssize_t
read (int fd, void *buf, size_t size)
{
  ssize_t nbyte;

  if (! isatty (fd))
    log_read (fd, size);

  nbyte = (ssize_t) syscall (SYS_read, fd, buf, size);

  if (! isatty (fd) && nbyte >= 0)
    horus_decrypt (fd, buf, (size_t) nbyte);

  return nbyte;
}

ssize_t
write (int fd, const void *buf, size_t size)
{
  ssize_t nbyte;
  void *buf2 = (void *) buf;

  if (! isatty (fd))
    log_write (fd, size);

  if (! isatty (fd))
    horus_encrypt (fd, buf2, size);

  nbyte = (ssize_t) syscall (SYS_write, fd, buf2, size);

  return nbyte;
}

int
close (int fd)
{
  log_close (fd);
  return (int) syscall (SYS_close, fd);
}

int
unlink (const char *path)
{
  log_unlink (path);
  return syscall (SYS_unlink, path);
}

int
dup2 (int fd, int fd2)
{
  log_dup2 (fd, fd2);
  return syscall (SYS_dup2, fd, fd2);
}


