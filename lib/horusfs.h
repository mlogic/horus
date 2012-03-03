#ifndef _HORUSFS_H_
#define _HORUSFS_H_

struct horusfs_state
{
  char *rootdir;
  char *kds_server;
};

#define HORUSFS_DATA ((struct horusfs_state *) fuse_get_context()->private_data)
#endif
