/*
 * Derived from :
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags --libs` fusexmp.c -o fusexmp
*/

#define FUSE_USE_VERSION 26


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include <horus_attr.h>
#include <horus_crypt.h>
#include <horus_key.h>
#include <horusfs.h>
#include <rpc/rpc.h>
#include <kds_rpc.h>


static void horusfs_fullpath (char fpath[PATH_MAX], const char *path)
{
  strcpy(fpath, HORUSFS_DATA->rootdir);
  strncat(fpath, path, PATH_MAX);
}

static int
xmp_getattr (const char *path, struct stat *stbuf)
{
  int res;
  char fpath[PATH_MAX];

  horusfs_fullpath(fpath, path);

  res = lstat (fpath, stbuf);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_access (const char *path, int mask)
{
  int res;

  res = access (path, mask);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_readlink (const char *path, char *buf, size_t size)
{
  int res;

  res = readlink (path, buf, size - 1);
  if (res == -1)
    return -errno;

  buf[res] = '\0';
  return 0;
}


static int
xmp_readdir (const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
  DIR *dp;
  struct dirent *de;
  char fpath[PATH_MAX];

  (void) offset;
  (void) fi;

  horusfs_fullpath(fpath, path);

  dp = opendir (fpath);
  if (dp == NULL)
    return -errno;

  while ((de = readdir (dp)) != NULL)
    {
      struct stat st;

      memset (&st, 0, sizeof (st));
      st.st_ino = de->d_ino;
      st.st_mode = de->d_type << 12;
      if (filler (buf, de->d_name, &st, 0))
        break;
    }

  closedir (dp);
  return 0;
}

static int
xmp_mknod (const char *path, mode_t mode, dev_t rdev)
{
  int res;

  /* On Linux this could just be 'mknod(path, mode, rdev)' but this
     is more portable */
  if (S_ISREG (mode))
    {
      res = open (path, O_CREAT | O_EXCL | O_WRONLY, mode);
      if (res >= 0)
        res = close (res);
    }
  else if (S_ISFIFO (mode))
    res = mkfifo (path, mode);
  else
    res = mknod (path, mode, rdev);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_mkdir (const char *path, mode_t mode)
{
  int res;

  res = mkdir (path, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_unlink (const char *path)
{
  int res;
  char fpath[PATH_MAX];


  horusfs_fullpath(fpath, path);

  res = unlink (fpath);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_rmdir (const char *path)
{
  int res;

  res = rmdir (path);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_symlink (const char *from, const char *to)
{
  int res;

  res = symlink (from, to);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_rename (const char *from, const char *to)
{
  int res;

  res = rename (from, to);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_link (const char *from, const char *to)
{
  int res;

  res = link (from, to);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_chmod (const char *path, mode_t mode)
{
  int res;

  res = chmod (path, mode);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_chown (const char *path, uid_t uid, gid_t gid)
{
  int res;

  res = lchown (path, uid, gid);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_truncate (const char *path, off_t size)
{
  int res;

  res = truncate (path, size);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_utimens (const char *path, const struct timespec ts[2])
{
  int res;
  struct timeval tv[2];

  tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;

  res = utimes (path, tv);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_open (const char *path, struct fuse_file_info *fi)
{
  int res;
  char fpath[PATH_MAX];

  horusfs_fullpath(fpath, path);

  res = open (fpath, fi->flags);
  if (res == -1)
    return -errno;

  close (res);
  return 0;
}


static int
xmp_read (const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
  int fd;
  int res;
  int err;
  char *ktype;
  char *kvalue;
  int decrypt = 0;

  char fpath[PATH_MAX];

  CLIENT *cl;
  struct key_request req;
  struct key_rtn *resp;
  char *server;
  int x,y;

  (void) fi;
  horusfs_fullpath(fpath, path);
  fd = open (fpath, O_RDONLY);
  if (fd == -1)
    return -errno;

  server = HORUSFS_DATA->kds_server;
  cl = clnt_create (server, KDS_RPC, KEYREQVERS, "udp");
  if (cl == NULL)
  {
    clnt_pcreateerror (server);
    abort ();
  }

  x = 9;
  y = offset / block_size[9];
 
  req.filename = fpath;
  req.ranges.ranges_len = 1 ;
  req.ranges.ranges_val = calloc(sizeof(struct range), 1);
  req.ranges.ranges_val[0].x = x;
  req.ranges.ranges_val[0].y = y;

  resp = keyreq_1 (&req, cl);
  if (resp == NULL)
  {
    clnt_perror (cl, "call failed:");
    abort();
  }
  clnt_destroy (cl);
  if (resp->keys.keys_val[0].err == 0)
    printf ("key = %s\n", resp->keys.keys_val[0].key);
  else
    printf ("key err = %s\n", strerror(resp->keys.keys_val[0].err));

  /*
e    err = lgetxattr(path, "user.horus.key.value", kvalue, KEY_VALUE_SIZE);
     if (err > 0)
     {
     err = lgetxattr(path, "user.horus.key.type", ktype, KEY_TYPE_SIZE);
     if (err > 0)
     decrypt = 1;
     } */
  res = pread (fd, buf, size, offset);
  if (res == -1)
    res = -errno;
  else
    {
      if (decrypt)
        horus_coding (0, (int) offset, buf, size, ktype, kvalue);
    }
  close (fd);
  return res;
}

static int
xmp_write (const char *path, const char *buf, size_t size,
           off_t offset, struct fuse_file_info *fi)
{
  int fd;
  int res;
  char *ktype = "master";
  char *kvalue = "nakul";
  char fpath[PATH_MAX];


  horusfs_fullpath(fpath, path);


  (void) fi;
  fd = open (fpath, O_WRONLY);
  if (fd == -1)
    return -errno;
  horus_coding (0, offset, buf, size, ktype, kvalue);

  res = pwrite (fd, buf, size, offset);
  if (res == -1)
    res = -errno;
  close (fd);
  return res;
}

static int
xmp_statfs (const char *path, struct statvfs *stbuf)
{
  int res;
  char fpath[PATH_MAX];


  horusfs_fullpath(fpath, path);

  res = statvfs (fpath, stbuf);
  if (res == -1)
    return -errno;

  return 0;
}

static int
xmp_release (const char *path, struct fuse_file_info *fi)
{
  /* Just a stub.  This method is optional and can safely be left
     unimplemented */

  (void) path;
  (void) fi;
  return 0;
}

static int
xmp_fsync (const char *path, int isdatasync, struct fuse_file_info *fi)
{
  /* Just a stub.  This method is optional and can safely be left
     unimplemented */

  (void) path;
  (void) isdatasync;
  (void) fi;
  return 0;
}

static void *xmp_init(struct fuse_conn_info *conn)
{
  return HORUSFS_DATA;
}

static void xmp_destroy(void *private_data)
{
  if (((struct horusfs_state *)private_data)->kds_server != NULL)
  {
    free(((struct horusfs_state *)private_data)->kds_server);
  }
}
#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int
xmp_setxattr (const char *path, const char *name, const char *value,
              size_t size, int flags)
{
  int res = lsetxattr (path, name, value, size, flags);

  if (res == -1)
    return -errno;
  return 0;
}

static int
xmp_getxattr (const char *path, const char *name, char *value, size_t size)
{
  int res = lgetxattr (path, name, value, size);

  if (res == -1)
    return -errno;
  return res;
}

static int
xmp_listxattr (const char *path, char *list, size_t size)
{
  int res = llistxattr (path, list, size);

  if (res == -1)
    return -errno;
  return res;
}

static int
xmp_removexattr (const char *path, const char *name)
{
  int res = lremovexattr (path, name);

  if (res == -1)
    return -errno;
  return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
  .getattr = xmp_getattr,
  .access = xmp_access,
  .readlink = xmp_readlink,
  .readdir = xmp_readdir,
  .mknod = xmp_mknod,
  .mkdir = xmp_mkdir,
  .symlink = xmp_symlink,
  .unlink = xmp_unlink,
  .rmdir = xmp_rmdir,
  .rename = xmp_rename,
  .link = xmp_link,
  .chmod = xmp_chmod,
  .chown = xmp_chown,
  .truncate = xmp_truncate,
  .utimens = xmp_utimens,
  .open = xmp_open,
  .read = xmp_read,
  .write = xmp_write,
  .statfs = xmp_statfs,
  .release = xmp_release,
  .fsync = xmp_fsync,
  .init = xmp_init,
  .destroy = xmp_destroy,
#ifdef HAVE_SETXATTR
  .setxattr = xmp_setxattr,
  .getxattr = xmp_getxattr,
  .listxattr = xmp_listxattr,
  .removexattr = xmp_removexattr,
#endif
};

void horusfs_usage()
{
    fprintf(stderr, "usage: horusfs kds_server rootDir mountPoint\n");
    abort();
}
int
main (int argc, char *argv[])
{
  int i,ret;
  struct horusfs_state *horusfs_data;
  umask (0);
  
  horusfs_data = calloc(sizeof(struct horusfs_state),1);
  if (horusfs_data == NULL)
  {
    fprintf(stderr, "Memory allocation failure!");
    abort(); 
  }

  for (i = 1; (i < argc) && (argv[i][0] == '-'); i++)
  {
    if (argv[i][1] == 'o')
      i++;
  }
  if ((argc - i) != 3)
    horusfs_usage();
  horusfs_data->kds_server = strdup(argv[i]);
  horusfs_data->rootdir = realpath(argv[i+1], NULL);
  argv[i] = argv[i+2];
  argc = argc - 2;

  return fuse_main (argc, argv, &xmp_oper, horusfs_data);
}
