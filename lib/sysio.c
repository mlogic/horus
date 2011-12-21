
#include <horus.h>

int
open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int fd = -1;

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
#ifdef ENABLE_HORUS
  horus_open (fd, path, oflag, mode);
#endif /*ENABLE_HORUS*/

  return fd;
}

int
socket (int domain, int type, int protocol)
{
  int fd = -1;

  fd = (int) syscall (SYS_socket, domain, type, protocol);
  log_socket (fd, domain, type, protocol);

  return fd;
}

ssize_t
read (int fd, void *buf, size_t size)
{
  int nbyte;
  off_t fdpos = 0;
  struct stat statbuf;

  fstat (fd, &statbuf);

  if (S_ISREG (statbuf.st_mode))
    fdpos = lseek (fd, 0, SEEK_CUR);

  nbyte = (int) syscall (SYS_read, fd, buf, size);

  if (S_ISREG (statbuf.st_mode))
    log_read (fd, fdpos, buf, (size_t) nbyte, size);

#ifdef ENABLE_HORUS
  /* Horus function only for regular file and successful read. */
  if (S_ISREG (statbuf.st_mode) && nbyte > 0)
    horus_read (fd, fdpos, buf, (size_t) nbyte);
#endif /*ENABLE_HORUS*/

  return (ssize_t) nbyte;
}

ssize_t
write (int fd, const void *buf, size_t size)
{
  int nbyte;
  off_t fdpos = 0;
  struct stat statbuf;

  fstat (fd, &statbuf);

  if (S_ISREG (statbuf.st_mode))
    fdpos = lseek (fd, 0, SEEK_CUR);

#ifdef ENABLE_HORUS
  if (S_ISREG (statbuf.st_mode))
    horus_write (fd, fdpos, (void *) buf, size);
#endif /*ENABLE_HORUS*/

  nbyte = (int) syscall (SYS_write, fd, buf, size);

  if (S_ISREG (statbuf.st_mode))
    log_write (fd, fdpos, (void *) buf, nbyte, size);

  return (ssize_t) nbyte;
}

int
close (int fd)
{
#ifdef ENABLE_HORUS
  horus_close (fd);
#endif /*ENABLE_HORUS*/
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
#ifdef ENABLE_HORUS
  horus_close (fd2);
#endif /*ENABLE_HORUS*/
  log_dup2 (fd, fd2);
  return syscall (SYS_dup2, fd, fd2);
}


