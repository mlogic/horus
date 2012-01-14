
#ifndef _HORUS_H_
#define _HORUS_H_

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
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#ifdef HAVE_OPENSSL
#include <openssl/sha.h>
#include <openssl/hmac.h>
#endif /*HAVE_OPENSSL*/

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif /*HAVE_PTHREAD*/

#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include <CommonCrypto/CommonHMAC.h>
#endif
#endif /*__APPLE__*/

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096

#include <minmax.h>

extern int debug;

void horus_init ();

int horus_open (int fd, const char *path, int flag, mode_t mode);
ssize_t horus_read (int fd, off_t fdpos, void *buf, size_t size);
ssize_t horus_write (int fd, off_t fdpos, void *buf, size_t size);
int horus_close (int fd);

#endif /*_HORUS_H_*/
