
extern int log_on;

void log_init ();
void log_open (int fd, const char *path, int oflag, mode_t mode);
void log_read (int fd, off_t fdpos, void *buf, size_t nbyte);
void log_write (int fd, off_t fdpos, void *buf, size_t nbyte);
void log_close (int fd);
void log_unlink (const char *path);
void log_dup2 (int fd, int fd2);

