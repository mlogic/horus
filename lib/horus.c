
#include <horus.h>

int debug = 0;
int is_init = 0;

/* libhorus tracks files by file descriptor. */
fd_set fdset;

void
horus_init ()
{
  if (! log_on)
    log_init ();
  FD_ZERO (&fdset);
  is_init++;
}

int
horus_open (int fd, const char *path, int oflag, mode_t mode)
{
  char *p, *filename, *match;

  if (! is_init)
    horus_init ();

  if (fd < 0)
    return -1;

  match = getenv ("HORUS_MATCH_FILE");
  if (! match)
    return 0;

  p = rindex (path, '/');
  filename = (p ? ++p : (char *) path);

  if (! strcasecmp (filename, match))
    FD_SET (fd, &fdset);

  return 0;
}

ssize_t
horus_read (int fd, off_t fdpos, void *buf, size_t size)
{
  char *ktype, *kvalue;

  if (! is_init)
    horus_init ();

  log_read (fd, fdpos, buf, size);

  if (FD_ISSET (fd, &fdset))
    {
      horus_get_key (&ktype, &kvalue);
      if (ktype && kvalue)
        horus_decrypt (fd, fdpos, buf, size, ktype, kvalue);
    }

  return 0;
}

ssize_t
horus_write (int fd, off_t fdpos, void *buf, size_t size)
{
  char *ktype, *kvalue;

  if (! is_init)
    horus_init ();

  log_write (fd, fdpos, buf, size);

  if (FD_ISSET (fd, &fdset))
    {
      horus_get_key (&ktype, &kvalue);
      if (ktype && kvalue)
        horus_encrypt (fd, fdpos, buf, size, ktype, kvalue);
    }

  return 0;
}

int
horus_close (int fd)
{
  if (! is_init)
    horus_init ();

  if (fd < 0)
    return -1;

  if (FD_ISSET (fd, &fdset))
    FD_CLR (fd, &fdset);

  return 0;
}


