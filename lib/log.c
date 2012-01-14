
#include <horus.h>

#include <log.h>

#define MESG_SIZE 4096

int log_on = 0;

void
log_init ()
{
#ifdef ENABLE_SYSLOG
  openlog ("horus", LOG_NOWAIT, LOG_LOCAL7);
  syslog (LOG_INFO, "openlog");
#endif /*ENABLE_SYSLOG*/
  log_on++;
}

void
log_open (int fd, const char *path, int oflag, mode_t mode)
{
  char mesg[MESG_SIZE];
  int ret;
  struct stat st;
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

  snprintf (mesg, sizeof (mesg),
    "open: fd: %d oflag: %#x mode: %#o path: %s",
    fd, oflag, mode, path);

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "%s", mesg);
#endif /*ENABLE_SYSLOG*/

  if (fd < 0)
    return;

  ret = fstat (fd, &st);
  if (ret != 0)
    {
#ifdef ENABLE_SYSLOG
      syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);
#endif /*ENABLE_SYSLOG*/
#ifdef ENABLE_STDERR
      fprintf (stderr, "%s(): fstat() failed: %s", __func__,
               strerror (errno));
#endif /*ENABLE_STDERR*/
    }

  snprintf (mesg, sizeof (mesg),
    "open: fd: %d dev: %d/%d mode: %#o ino: %lld",
    fd, (int) st.st_rdev, (int) st.st_dev, (int) st.st_mode,
    (unsigned long long) st.st_ino);

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "%s", mesg);
#endif /*ENABLE_SYSLOG*/
}

void
log_socket (int fd, int domain, int type, int protocol)
{
  char mesg[MESG_SIZE];
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  snprintf (mesg, sizeof (mesg),
    "socket: fd: %d domain: %d type: %d protocol: %d",
    fd, domain, type, protocol);

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "%s", mesg);
#endif /*ENABLE_SYSLOG*/
}

void
log_read (int fd, off_t fdpos, void *buf, size_t nbyte, size_t size)
{
  char mesg[MESG_SIZE];
  int ret;
  struct stat st = { 0 };
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

  ret = fstat (fd, &st);
  if (ret != 0)
    {
#ifdef ENABLE_SYSLOG
      syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);
#endif /*ENABLE_SYSLOG*/
#ifdef ENABLE_STDERR
      fprintf (stderr, "%s(): fstat() failed: %s", __func__,
               strerror (errno));
#endif /*ENABLE_STDERR*/
    }

  snprintf (mesg, sizeof (mesg),
    "read: fd: %d bytes: %d/%d file: %d/%d",
    fd, (int) nbyte, (int) size, (int) fdpos, (int) st.st_size);

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "%s", mesg);
#endif /*ENABLE_SYSLOG*/
}

void
log_write (int fd, off_t fdpos, void *buf, size_t nbyte, size_t size)
{
  char mesg[MESG_SIZE];
  int ret;
  struct stat st = { 0 };
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

  ret = fstat (fd, &st);
  if (ret != 0)
    {
#ifdef ENABLE_SYSLOG
      syslog (LOG_INFO, "%s(): fstat() failed: %m", __func__);
#endif /*ENABLE_SYSLOG*/
#ifdef ENABLE_STDERR
      fprintf (stderr, "%s(): fstat() failed: %s", __func__,
               strerror (errno));
#endif /*ENABLE_STDERR*/
    }

  snprintf (mesg, sizeof (mesg),
    "write: fd: %d bytes: %d/%d file:%d/%d",
    fd, (int) nbyte, (int) size, (int) fdpos, (int) st.st_size);

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: %s\n", (int) pid, mesg);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "%s", mesg);
#endif /*ENABLE_SYSLOG*/
}

void
log_close (int fd)
{
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: close: fd: %d\n", (int) pid, fd);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "close: fd: %d", fd);
#endif /*ENABLE_SYSLOG*/
}

void
log_unlink (const char *path)
{
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: unlink: %s\n", (int) pid, path);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "unlink: %s", path);
#endif /*ENABLE_SYSLOG*/
}

void
log_dup2 (int fd, int fd2)
{
#ifdef ENABLE_STDERR
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

#ifdef ENABLE_STDERR
  pid = getpid ();
  fprintf (stderr, "pid: %d: dup2: %d -> %d\n", (int) pid, fd, fd2);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  syslog (LOG_INFO, "dup2: %d -> %d", fd, fd2);
#endif /*ENABLE_SYSLOG*/
}

void
log_mesg (int priority, char *message, ...)
{
#ifdef ENABLE_SYSLOG
  va_list ap_syslog;
#endif /*ENABLE_SYSLOG*/
#ifdef ENABLE_STDERR
  va_list ap_stderr;
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

#ifdef ENABLE_STDERR
  pid = getpid ();
  va_start (ap_stderr, message);
  fprintf (stderr, "pid: %d: ", (int) pid);
  vfprintf (stderr, message, ap_stderr);
  va_end (ap_stderr);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  va_start (ap_syslog, message);
  vsyslog (priority, message, ap_syslog);
  va_end (ap_syslog);
#endif /*ENABLE_SYSLOG*/
}

void
log_error (char *message, ...)
{
#ifdef ENABLE_SYSLOG
  va_list ap_syslog;
#endif /*ENABLE_SYSLOG*/
#ifdef ENABLE_STDERR
  va_list ap_stderr;
  pid_t pid;
#endif /*ENABLE_STDERR*/

  if (! log_on)
    log_init ();

#ifdef ENABLE_STDERR
  pid = getpid ();
  va_start (ap_stderr, message);
  fprintf (stderr, "pid: %d: ", (int) pid);
  vfprintf (stderr, message, ap_stderr);
  va_end (ap_stderr);
#endif /*ENABLE_STDERR*/
#ifdef ENABLE_SYSLOG
  va_start (ap_syslog, message);
  vsyslog (LOG_ERR, message, ap_syslog);
  va_end (ap_syslog);
#endif /*ENABLE_SYSLOG*/
}

