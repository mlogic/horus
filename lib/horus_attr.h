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

#define HORUS_FATTR_NAME "user.horus.config.new"
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


#define HORUS_EA_NAME "user.horus.config"
#define HORUS_EA_SIZE 1240
#define HORUS_EA_MAX_ENTRIES 100
#define HORUS_MASTERKEY_LEN 32


/* horus_ea_config_entry specifies range allowed for a client with IP addr. */
struct horus_ea_config_entry
{
  unsigned char addr[4];        /* Client IP address */
  unsigned char start_block_num[4];     /* Start block number */
  unsigned char end_block_num[4];       /* End block number */
};

/* horus_ea_config is the actual extended attribute which stores all the configuration
   Size of it is 4 + 4 + 32 + 100*12 = 1240 bytes. No specific reason to keep it this size. */

struct horus_ea_config
{
  unsigned char reserved[4];    /* Maybe store magic number or some other bits */
  unsigned char entry_count[4];
  unsigned char kht_depth[4];
  unsigned char branching_factor[4];
  unsigned char master_key[HORUS_MASTERKEY_LEN];
  struct horus_ea_config_entry entry_table[100];        /* 100 * 12 = 1200 bytes. Should be sufficient for testing */
};

/*
 * horus_set_fattr_masterkey
 * In: 
 * fd - File Descriptor of file
 * key - Masterkey to be set
 * Out:
 * return code
*/

int horus_set_fattr_masterkey (int fd, char *key);

/*
 * horus_get_fattr_masterkey
 * In:
 * fd     - File Descriptor of file
 * bufsiz - size of buffer
 * 
 * Out:
 * buf - Masterkey
 * return code
*/
int horus_get_fattr_masterkey (int fd, char *buf, int bufsiz);

/*
 * horus_set_fattr_client
 * In:
 * fd     - File Descriptor of file
 * client - IP address of client
 * start  - Start block number
 * end    - End block number
 * Out:
 * return code
*/

int
horus_set_fattr_client (int fd, struct in_addr *client,
                        u_int32_t start, u_int32_t end);

/*
 * horus_get_fattr_client
 * In:
 * fd      - File Descriptor of File
 * client  - IP address of client
 * Out
 * start   - Start block number
 * end     - End block number
*/

int
horus_get_fattr_client (int fd, struct in_addr *client,
                        u_int32_t * start, u_int32_t * end);
int horus_ea_config_show (char *path);

int
horus_ea_config_add_entry (char *path, int in_fd, struct in_addr ip,
                           uint32_t start_block, uint32_t end_block);

int horus_ea_config_masterkey (char *path, int in_fd, char *key);

int horus_get_fattr (int fd, struct horus_ea_config *config);

int horus_get_fattr_config_client (struct horus_ea_config *config,
                                   struct in_addr *client, u_int32_t * start,
                                   u_int32_t * end);

int horus_get_fattr_masterkey_config (struct horus_ea_config *config,
                                      char *buf, int bufsiz);


int horus_get_file_config (int fd, struct horus_file_config *config);
int horus_set_file_config (int fd, struct horus_file_config *config);
int horus_get_master_key (int fd, char *buf, int bufsize);
int horus_set_master_key (int fd, char *buf);
int horus_set_kht_block_size (int fd, int level, unsigned int size);

int horus_get_client_range (struct horus_file_config *c,
                            struct in_addr *addr,
                            unsigned int *sblock, unsigned int *eblock);
#endif
