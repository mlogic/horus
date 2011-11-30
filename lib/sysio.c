
#include <horus.h>

#define MESG_SIZE 4096

extern int errno;

int syslog_initialized = 0;

void
syslog_init ()
{
  openlog ("horus", LOG_NOWAIT, LOG_LOCAL7);
  syslog (LOG_INFO, "openlog.");
  syslog_initialized++;
}

ssize_t
read (int fd, void *buf, size_t nbyte)
{
  int ret;
  pid_t pid;
  struct stat st;
  off_t offset;
  char mesg[MESG_SIZE];

  if (! syslog_initialized)
    syslog_init ();

  ret = fstat (fd, &st);
  if (ret != 0)
    syslog (LOG_INFO, "fstat() failed: %m");

  pid = getpid ();
  offset = lseek (fd, 0, SEEK_CUR);

  if (ret == 0)
    {
      snprintf (mesg, sizeof (mesg),
        "read: %d(+%d)/%d rdev: %d, dev: %d, ino: %llx",
        (int) offset, (int) nbyte,
        (int) st.st_size, (int) st.st_rdev, (int) st.st_dev,
        (unsigned long long) st.st_ino);
    }
  else
    {
      snprintf (mesg, sizeof (mesg),
        "read: %d(+%d)",
        (int) offset, (int) nbyte);
    }

  //fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
  syslog (LOG_INFO, "%s", mesg);

  return (ssize_t) syscall (SYS_read, fd, buf, nbyte);
}

ssize_t
write (int fd, const void *buf, size_t nbyte)
{
  int ret;
  pid_t pid;
  struct stat st;
  off_t offset;
  char mesg[MESG_SIZE];

  if (! syslog_initialized)
    syslog_init ();

  ret = fstat (fd, &st);
  if (ret != 0)
    syslog (LOG_INFO, "fstat() failed: %m");

  pid = getpid ();
  offset = lseek (fd, 0, SEEK_CUR);

  if (ret == 0)
    {
      snprintf (mesg, sizeof (mesg),
        "write: %d(+%d)/%d rdev: %d, dev: %d, ino: %llx",
        (int) offset, (int) nbyte,
        (int) st.st_size, (int) st.st_rdev, (int) st.st_dev,
        (unsigned long long) st.st_ino);
    }
  else
    {
      snprintf (mesg, sizeof (mesg),
        "write: %d(+%d)",
        (int) offset, (int) nbyte);
    }

  //fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
  syslog (LOG_INFO, "%s", mesg);

  return (ssize_t) syscall (SYS_write, fd, buf, nbyte);
}

int
unlink (const char *path)
{
  pid_t pid;
  pid = getpid ();
  if (! syslog_initialized)
    syslog_init ();
  //fprintf (stderr, "pid: %d: unlink %s.\n", (int) pid, path);
  syslog (LOG_INFO, "unlink %s.", path);
  return syscall (SYS_unlink, path);
}

