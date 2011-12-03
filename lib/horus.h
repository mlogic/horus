
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /*HAVE_CONFIG_H*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <syslog.h>
#include <sys/select.h>

#ifdef HAVE_OPENSSL
#include <openssl/sha.h>
#include <openssl/hmac.h>
#endif /*HAVE_OPENSSL*/

#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include <CommonCrypto/CommonHMAC.h>
#endif
#endif /*__APPLE__*/

#include "log.h"

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif /*MIN*/

extern int debug;
extern unsigned int block_size[];

void horus_init ();
void key_to_value (char *key, int *key_len, char *string);
void key_to_string (char *buf, int size, char *key, int key_len);

int snprint_hex (char *buf, int bufsiz, char *key, int size);
void print_hex (char *prefix, char *key, int size);
char *print_key (char *key, int key_len);

char *
block_key (char *key, int *key_len,
           char *parent, int parent_len, int x, int y);

int
horus_key_from_to (char *key, int *key_len, int x, int y,
                   char *parent, int parent_len, int parent_x, int parent_y);

int
horus_key_by_master (char *key, int *key_len, int x, int y,
                     char *master, int master_len);

void
horus_key (char *key, int *key_len, int filepos,
           char *ktype, char *kvalue);

void
horus_get_key (char **ktype, char **kvalue);

void
horus_coding (int fd, int fdpos, char *buf, size_t nbyte,
              char *ktype, char *kvalue);

int horus_open (int fd, const char *path, int flag, mode_t mode);
ssize_t horus_read (int fd, off_t fdpos, void *buf, size_t size);
ssize_t horus_write (int fd, off_t fdpos, void *buf, size_t size);
int horus_close (int fd);

