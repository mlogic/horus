
#include <horus.h>

#define MESG_SIZE 4096

int log_on = 0;

void
log_init ()
{
  openlog ("horus", LOG_NOWAIT, LOG_LOCAL7);
  syslog (LOG_INFO, "openlog");
  log_on++;
}

void
log_open (int fd, const char *path, int oflag, mode_t mode)
{
  char mesg[MESG_SIZE];
  int ret;
  struct stat st;
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

  snprintf (mesg, sizeof (mesg),
    "open: fd: %d oflag: %#x mode: %#o path: %s",
    fd, oflag, mode, path);

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "%s", mesg);

  if (fd < 0)
    return;

  ret = fstat (fd, &st);
  if (ret != 0)
    syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);

  snprintf (mesg, sizeof (mesg),
    "open: fd: %d dev: %d/%d mode: %#o ino: %lld",
    fd, (int) st.st_rdev, (int) st.st_dev, (int) st.st_mode,
    (unsigned long long) st.st_ino);

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "%s", mesg);
}

void
log_read (int fd, size_t nbyte)
{
  char mesg[MESG_SIZE];
  off_t offset;
  int ret;
  struct stat st = { 0 };
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

  offset = lseek (fd, 0, SEEK_CUR);
  ret = fstat (fd, &st);
  if (ret != 0)
    syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);

  snprintf (mesg, sizeof (mesg),
    "read: fd: %d bytes: %d(+%d)/%d",
    fd, (int) offset, (int) nbyte, (int) st.st_size);

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "%s", mesg);
}

void
log_write (int fd, size_t nbyte)
{
  char mesg[MESG_SIZE];
  off_t offset;
  int ret;
  struct stat st = { 0 };
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

  offset = lseek (fd, 0, SEEK_CUR);
  ret = fstat (fd, &st);
  if (ret != 0)
    syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);

  snprintf (mesg, sizeof (mesg),
    "write: fd: %d bytes: %d(+%d)/%d",
    fd, (int) offset, (int) nbyte, (int) st.st_size);

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "%s", mesg);
}

void
log_close (int fd)
{
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: close: fd: %d\n", (int) pid, fd);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "close: fd: %d", fd);
}

void
log_unlink (const char *path)
{
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: unlink: %s\n", (int) pid, path);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "unlink: %s", path);
}

void
log_dup2 (int fd, int fd2)
{
#ifdef LOG_STDERR
  pid_t pid;
#endif /*LOG_STDERR*/

  if (! log_on)
    log_init ();

#ifdef LOG_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: dup2: %d -> %d\n", (int) pid, fd, fd2);
#endif /*LOG_STDERR*/
  syslog (LOG_INFO, "dup2: %d -> %d", fd, fd2);
}

