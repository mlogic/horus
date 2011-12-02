
#include <horus.h>

int fdesc = -1;

int
open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int fd = 0;
  char filename[1024];
  char *p, *cond;

  strncpy (filename, path, sizeof (filename));
  p = rindex (filename, '/');
  if (p)
    p++;
  else
    p = (char *) path;
  cond = getenv ("HORUS_MATCH_FILE");

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

  if (p && cond && ! strcasecmp (p, cond))
    {
      fdesc = fd;
    }
  syslog (LOG_INFO, "path: %s, cond: %s, fd: %d, fdesc: %d",
          p, cond, fd, fdesc);

  return fd;
}

ssize_t
read (int fd, void *buf, size_t size)
{
  int nbyte;
  off_t fdpos;
  char buf1[9000], buf2[9000];

  memset (buf1, 0, sizeof (buf1));
  memset (buf2, 0, sizeof (buf2));

  if (! isatty (fd))
    log_read (fd, size);

  fdpos = lseek (fd, 0, SEEK_CUR);

  nbyte = (int) syscall (SYS_read, fd, buf, size);

  if (fdesc == fd)
    memcpy (buf1, buf, MIN (size, sizeof (buf1)));

  if (! isatty (fd) && fdesc == fd && nbyte >= 0)
    horus_coding (fd, buf, fdpos, (size_t) nbyte);

  if (fdesc == fd)
    memcpy (buf2, buf, MIN (size, sizeof (buf2)));

  if (fdesc == fd)
    syslog (LOG_INFO, "read: before: %#02x, after: %#02x",
            (unsigned char) buf1[0],
            (unsigned char) buf2[0]);

  return (ssize_t) nbyte;
}

ssize_t
write (int fd, const void *buf, size_t size)
{
  int nbyte;
  char buf1[9000], buf2[9000];
  off_t fdpos;

  memset (buf1, 0, sizeof (buf1));
  memset (buf2, 0, sizeof (buf2));

  if (! isatty (fd))
    log_write (fd, size);

  fdpos = lseek (fd, 0, SEEK_CUR);

  if (fdesc == fd)
    memcpy (buf1, buf, MIN (size, sizeof (buf2)));

  if (! isatty (fd) && fdesc == fd)
    horus_coding (fd, (void *) buf, fdpos, size);

  if (fdesc == fd)
    memcpy (buf2, buf, MIN (size, sizeof (buf2)));

  if (fdesc == fd)
    syslog (LOG_INFO, "read: before: %#02x, after: %#02x",
            (unsigned char) buf1[0],
            (unsigned char) buf2[0]);

  nbyte = (int) syscall (SYS_write, fd, buf, size);

  return (ssize_t) nbyte;
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


