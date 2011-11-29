#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define MESG_SIZE 4096

extern int errno;

ssize_t
read (int fd, void *buf, size_t nbyte)
{
  int ret;
  pid_t pid;
  struct stat st;
  off_t offset;
  char mesg[MESG_SIZE];

  ret = fstat (fd, &st);
  if (ret != 0)
    fprintf (stderr, "fstat() failed: %s\n", strerror (errno));

  pid = getpid ();
  offset = lseek (fd, 0, SEEK_CUR);

  if (ret == 0)
    {
      snprintf (mesg, sizeof (mesg),
        "pid: %d: read: %d(+%d)/%d rdev: %d, dev: %d, ino: %llx",
        (int) pid, (int) offset, (int) nbyte,
        (int) st.st_size, (int) st.st_rdev, (int) st.st_dev,
        (unsigned long long) st.st_ino);
    }
  else
    {
      snprintf (mesg, sizeof (mesg),
        "pid: %d: read: %d(+%d)",
        (int) pid, (int) offset, (int) nbyte);
    }

  fprintf (stderr, "%s\n", mesg);

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

  ret = fstat (fd, &st);
  if (ret != 0)
    fprintf (stderr, "fstat() failed: %s\n", strerror (errno));

  offset = lseek (fd, 0, SEEK_CUR);

  if (ret == 0)
    {
      snprintf (mesg, sizeof (mesg),
        "pid: %d: write: %d(+%d)/%d rdev: %d, dev: %d, ino: %llx",
        (int) pid, (int) offset, (int) nbyte,
        (int) st.st_size, (int) st.st_rdev, (int) st.st_dev,
        (unsigned long long) st.st_ino);
    }
  else
    {
      snprintf (mesg, sizeof (mesg),
        "pid: %d: write: %d(+%d)",
        (int) pid, (int) offset, (int) nbyte);
    }

  fprintf (stderr, "%s\n", mesg);

  return (ssize_t) syscall (SYS_write, fd, buf, nbyte);
}

int
unlink (const char *path)
{
  fprintf (stderr, "Unlink %s.\n", path);
  return syscall (SYS_unlink, path);
}

