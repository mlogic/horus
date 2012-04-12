#ifndef _HORUS_ATTR_H_
#define _HORUS_ATTR_H_

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <stdint.h>

#define HORUS_FATTR_VERSION "v0.1"
#define HORUS_FATTR_NAME "user.horus.config." HORUS_FATTR_VERSION
#define HORUS_MAX_MASTERKEY_LEN 32
#define HORUS_MAX_KHT_DEPTH 16
#define HORUS_MAX_CLIENT_ENTRY 128

struct horus_client_range
{
  struct in_addr prefix;
  int prefixlen;
  unsigned int start;
  unsigned int end;
};

#define IS_HORUS_CLIENT_RANGE_EMPTY(clientp) \
  ((clientp)->start == 0 && (clientp)->end == 0)

struct horus_file_config
{
  char master_key[HORUS_MAX_MASTERKEY_LEN];
  unsigned int kht_block_size[HORUS_MAX_KHT_DEPTH];
  struct horus_client_range client_range[HORUS_MAX_CLIENT_ENTRY];
};

int horus_get_file_config (int fd, struct horus_file_config *config);
int horus_set_file_config (int fd, struct horus_file_config *config);
int horus_get_master_key (int fd, char *buf, int bufsize);
int horus_set_master_key (int fd, char *buf);
int horus_set_kht_block_size (int fd, int level, unsigned int size);

int horus_add_client_range (int fd, struct in_addr *prefix, int prefixlen,
                            unsigned int sblock, unsigned int eblock);
int horus_get_client_range (struct horus_file_config *c,
                            struct in_addr *addr,
                            unsigned int *sblock, unsigned int *eblock);
int horus_clear_client_range (int fd);

int horus_delete_file_config (int fd);

int horus_is_valid_config (struct horus_file_config *c);

#endif

