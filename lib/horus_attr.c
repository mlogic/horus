/*
 * Horus File Attributes Utils
 *
 * Copyright (c) 2012, University of California, Santa Cruz, CA, USA.
 * Developers:
 *  Nakul Dhotre <nakul@soe.ucsc.edu>
 *  Yan Li <yanli@ucsc.edu>
 *  Yasuhiro Ohara <yasu@soe.ucsc.edu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <sys/xattr.h>
#include <sys/types.h>
#include <log.h>
#include <horus_attr.h>

inline ssize_t
_fgetxattr (int fd, const char *name, void *value, size_t size)
{
#ifdef __APPLE__
  return fgetxattr (fd, name, value, size, 0, 0);
#else /* For Linux */
  return fgetxattr (fd, name, value, size);
#endif
}

inline ssize_t
_fsetxattr (int fd, const char *name, void *value, size_t size, int flags)
{
#ifdef __APPLE__
  return fsetxattr (fd, name, value, size, 0, 0);
#else /* For Linux */
  return fsetxattr (fd, name, value, size, flags);
#endif
}

inline ssize_t
_lgetxattr (const char *path, const char *name, void *value, size_t size)
{
#ifdef __APPLE__
  return getxattr (path, name, value, size, 0, XATTR_NOFOLLOW);
#else /* For Linux */
  return lgetxattr (path, name, value, size);
#endif
}

inline ssize_t
_fremovexattr (int fd, const char *name, int flags)
{
#ifdef __APPLE__
  return fremovexattr (fd, name, 0);
#else /* For Linux */
  return fremovexattr (fd, name);
#endif
}

int
horus_get_file_config (int fd, struct horus_file_config *config)
{
  int ret;

  ret = _fgetxattr (fd, HORUS_FATTR_NAME, (void *) config,
                    sizeof (struct horus_file_config));
  if (ret < 0)
    log_error ("fgetxattr() failed in %s: %s", __FUNCTION__, strerror (errno));
  return ret;
}

int
horus_set_file_config (int fd, struct horus_file_config *config)
{
  int ret;

  ret = _fsetxattr (fd, HORUS_FATTR_NAME, (void *) config,
                    sizeof (struct horus_file_config), 0);
  if (ret < 0)
    log_error ("fsetxattr() failed in %s: %s", __FUNCTION__, strerror (errno));
  return ret;
}

int
horus_get_master_key (int fd, char *buf, int bufsize)
{
  int ret;
  struct horus_file_config c;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    return ret;
  ret = snprintf (buf, bufsize, "%s", c.master_key);
  return ret;
}

int
horus_set_master_key (int fd, char *buf)
{
  int ret;
  struct horus_file_config c;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    memset (&c, 0, sizeof (struct horus_file_config));
  ret = snprintf (c.master_key, sizeof (c.master_key), "%s", buf);
  horus_set_file_config (fd, &c);
  return ret;
}

int
horus_set_kht_block_size (int fd, int level, unsigned int size)
{
  int ret;
  struct horus_file_config c;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    memset (&c, 0, sizeof (struct horus_file_config));
  c.kht_block_size[level] = size;
  horus_set_file_config (fd, &c);
  return ret;
}

int
horus_add_client_range (int fd, struct in_addr *prefix, int prefixlen,
                        unsigned int sblock, unsigned int eblock)
{
  int ret;
  int i;
  struct horus_file_config c;
  struct horus_client_range *match;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    memset (&c, 0, sizeof (struct horus_file_config));

  match = NULL;
  for (i = 0; i < HORUS_MAX_CLIENT_ENTRY; i++)
    {
      struct horus_client_range *p = &c.client_range[i];

      if (IS_HORUS_CLIENT_RANGE_EMPTY (p))
        {
          match = p;
          break;
        }
    }
  if (match)
    {
      match->prefix = *prefix;
      match->prefixlen = prefixlen;
      match->start = sblock;
      match->end = eblock;
    }
  horus_set_file_config (fd, &c);
  return ret;
}

int
horus_get_client_range (struct horus_file_config *c, struct in_addr *addr,
                        unsigned int *sblock, unsigned int *eblock)
{
  int i;
  struct horus_client_range *p, *match = NULL;

  for (i = 0; i < HORUS_MAX_CLIENT_ENTRY; i++)
    {
      p = &c->client_range[i];
      if (! IS_HORUS_CLIENT_RANGE_EMPTY (p) &&
          p->prefix.s_addr == addr->s_addr)
        {
          match = p;
          break;
        }
    }
  if (match)
    {
      *sblock = match->start;
      *eblock = match->end;
      return 0;
    }
  else
    return ENOENT;
}

int
horus_clear_client_range (int fd)
{
  int ret;
  struct horus_file_config c;

  ret = horus_get_file_config (fd, &c);
  if (ret < 0)
    memset (&c, 0, sizeof (struct horus_file_config));
  else
    memset (&c.client_range, 0, sizeof (c.client_range));
  horus_set_file_config (fd, &c);
  return ret;
}

int
horus_delete_file_config (int fd)
{
  int ret;

  ret = _fremovexattr (fd, HORUS_FATTR_NAME, 0);
  if (ret < 0)
    log_error ("fsetxattr() failed in %s: %s", __FUNCTION__, strerror (errno));
  return ret;
}

int
horus_is_valid_config (struct horus_file_config *c)
{
  if (c->kht_block_size[0] != 0)
    return 1;
  return 0;
}


