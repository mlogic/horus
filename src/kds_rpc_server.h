#ifndef _KDS_RPC_SERVER_H_
#define _KDS_RPC_SERVER_H_

#include <horus_attr.h>
#include <uthash.h>
/* Cache size */
#define MAX_EA_CACHE 10000

typedef struct
{
  uint64_t inode_num;           /* Key for searching EA */
  struct horus_ea_config config;
  UT_hash_handle hh;
} EA_CACHE_ENTRY;

EA_CACHE_ENTRY *ea_cache = NULL;
static int ea_cache_count = 0;

int ea_cache_add (uint64_t inode_num, struct horus_ea_config *config_ptr);
int ea_cache_find (uint64_t inode_num, EA_CACHE_ENTRY ** temp_ptr);

#endif
