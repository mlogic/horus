
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
  horus_open (fd, path, oflag, mode);

  return fd;
}

ssize_t
read (int fd, void *buf, size_t size)
{
  int nbyte;
  off_t fdpos;

  fdpos = lseek (fd, 0, SEEK_CUR);

  if (! isatty (fd))
    log_read (fd, fdpos, buf, (size_t) nbyte);

  nbyte = (int) syscall (SYS_read, fd, buf, size);

  if (! isatty (fd))
    horus_read (fd, fdpos, buf, (size_t) nbyte);

  return (ssize_t) nbyte;
}

ssize_t
write (int fd, const void *buf, size_t size)
{
  int nbyte;
  off_t fdpos;

  fdpos = lseek (fd, 0, SEEK_CUR);

  if (! isatty (fd))
    horus_write (fd, fdpos, (void *) buf, size);

  if (! isatty (fd))
    log_write (fd, fdpos, (void *) buf, size);

  nbyte = (int) syscall (SYS_write, fd, buf, size);

  return (ssize_t) nbyte;
}

int
close (int fd)
{
  horus_close (fd);
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
  horus_close (fd2);
  log_dup2 (fd, fd2);
  return syscall (SYS_dup2, fd, fd2);
}


