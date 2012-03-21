
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

/* Optimization Functions */
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/* Actual branch factor is 2^BRANCH_FACTOR_BITS, i.e., 2^2 = 4. */
#define BRANCH_FACTOR_BITS     2
#define MIN_CHUNK_SIZE      4096
#define HORUS_BLOCK_SIZE (MIN_CHUNK_SIZE)
#define HORUS_KEY_LEN    (160/8)

#include <minmax.h>

extern int debug;
extern int horus_debug;
extern int horus_verbose;

unsigned long long
canonical_byte_size (char *notation, char **endptr);

void horus_init ();

int horus_open (const int fd, const char *path, int flag, mode_t mode);
ssize_t horus_pread (int fd, void *buf, size_t size, off_t fdpos);
ssize_t horus_pwrite (int fd, void *buf, size_t size, off_t fdpos);
ssize_t horus_read (const int fd, void *buf, size_t nbyte);
ssize_t horus_write (const int fd, void *buf, size_t nbyte);
int horus_close (const int fd);
/** Add Kx,y
 *  If key_len > HORUS_KEY_LEN, key will be truncated to HORUS_KEY_LEN
 */
int horus_add_key (const int fd, const void *key, size_t key_len, const int x, const int y);
/** Set KHT properties
 *  \param fd file description
 *  \param depth depth of KHT
 *  \param leaf_block_size block size of leaf level
 *  \param branching_factor an array of each level's branching factor
 */
void horus_set_kht (const int fd, int depth, size_t leaf_block_size, const int *branching_factor);
/** Get K(x,y)
 *  \param out_key a output buffer for key of length HORUS_KEY_LENGH
 * You can only get keys lower in the keyed hash tree than what you
 * already have */
int horus_get_key (const int fd, void *out_key, const int x, const int y);
/* Get the key for block by ID
 * \param out_key A buffer of at least size HORUS_KEY_LEN
 */
int horus_get_leaf_block_key (const int fd, void *out_key, size_t block_id);
/* Get block size at level x */
ssize_t horus_get_block_size (const int fd, const int x);

#endif /*_HORUS_H_*/
