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
_fgetxattr (int fd, const char *name,
	    void *value, size_t size)
{
#ifdef __APPLE__
  return fgetxattr (fd, name, value, size, 0, 0);
#else /* For Linux */
  return fgetxattr (fd, name, value, size);
#endif
}

inline ssize_t
_fsetxattr (int fd, const char *name,
	    void *value, size_t size, int flags)
{
#ifdef __APPLE__
  return fsetxattr (fd, name, value, size, 0, 0);
#else /* For Linux */
  return fsetxattr (fd, name, value, size, flags);
#endif
}

inline ssize_t
_lgetxattr(const char *path, const char *name,
	   void *value, size_t size)
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

/* OLD CODE */

int
horus_ea_config_init (struct horus_ea_config *config)
{
  bzero (config, sizeof (struct horus_ea_config));
  return 0;
}


int
horus_ea_config_masterkey (char *path, int in_fd, char *key)
{
  int fd, res = 0;
  struct horus_ea_config config;

  if (strlen (key) > HORUS_MASTERKEY_LEN)
    {
      fprintf (stderr, "Master key is too big\n");
      return -1;
    }

  if (in_fd == -1 && path != NULL)
    {
      fd = open (path, O_RDWR);
      if (fd == -1)
        return -errno;
    }
  else if (in_fd == -1 && path == NULL)
    {
      fprintf (stderr, "path and fd both not specified!\n");
      return -1;
    }
  else
    {
      fd = in_fd;
    }
  res =
    _fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE);
  if (res == -1)
    {
      horus_ea_config_init (&config);
    }

  strncpy ((char*)config.master_key, key, strlen(key));

 //set_attr:
  res =
    _fsetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);
  if (res == -1)
    return -errno;
  if (in_fd == -1)
    {
      close (fd);               // If we opened the file we close otherwise not.
    }
  return 0;

}

int
horus_ea_config_add_entry (char *path, int in_fd, struct in_addr ip,
                           uint32_t start_block, uint32_t end_block)
{
  uint32_t count;
  struct horus_ea_config config;
  int fd, res = 0;

  if (in_fd == -1)
    {
      fd = open (path, O_RDWR);
      if (fd == -1)
        return -errno;
    }
  else
    fd = in_fd;

  res =
    _fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE);

  /* Error here most probably means this is a new file and we are setting EA for first time. 
   * TODO: We will have to check for operation not permitted error */
  if (res == -1)
    horus_ea_config_init (&config);


  memcpy (&count, config.entry_count, 4);
  memcpy (config.entry_table[count].addr, &ip.s_addr, 4);       /* TODO: Need to improve this code. Its not portable */
  memcpy (config.entry_table[count].start_block_num, &start_block, 4);
  memcpy (config.entry_table[count].end_block_num, &end_block, 4);
  count++;
  memcpy (config.entry_count, &count, 4);



 //set_attr:
  res =
    _fsetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);
  if (res == -1)
    return -errno;
  if (in_fd == -1)
    {
      close (fd);
    }
  return res;
}



int
horus_ea_config_show (char *path)
{
  int count, i;
  uint32_t start_block, end_block;
  int fd, res;
  struct in_addr ip;
  struct horus_ea_config config;
  char *ipaddr = NULL;

  fd = open (path, O_RDONLY);
  if (fd == -1)
    return -errno;

  res =
    _lgetxattr (path, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE);

  /* If we just return from here we can create a get_config API function */
  if (res == -1)
    return -errno;
  memcpy (&count, config.entry_count, 4);

  if (strlen (config.master_key) > 0)
    printf ("Master key = %s\n", config.master_key);
  else
    printf ("Master key not set\n");
  printf ("Number of entries = %d\n", count);
  for (i = 0; i < count; i++)
    {
      memcpy (&start_block, config.entry_table[i].start_block_num, 4);
      memcpy (&end_block, config.entry_table[i].end_block_num, 4);
      memcpy (&ip.s_addr, config.entry_table[i].addr, 4);
      printf ("Entry %d - IP %s  Start Block %d End Block %d\n",
              i, inet_ntoa (ip), start_block, end_block);
    }
  return 0;
}


int
horus_set_fattr_masterkey (int fd, char *key)
{
  return horus_ea_config_masterkey (NULL, fd, key);
}

int
horus_get_fattr_masterkey (int fd, char *buf, int bufsiz)
{
  int res, len;
  struct horus_ea_config config;

  res =
    _fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE);

  if (res == -1)
    return -errno;

  len = strlen ((char*)config.master_key);
  if (len + 1 > bufsiz)
    return -ENOSPC;

  if (len > 0)
    memcpy (buf, config.master_key, len + 1);
  else
    *buf = '\0';

  return (strlen (config.master_key));
}


int horus_get_fattr_masterkey_config (struct horus_ea_config *config,
                                      char *buf, int bufsiz)
{
  int len;
  len = strlen (config->master_key);
  if (len + 1 > bufsiz)
    return -ENOSPC;

  if (len > 0)
    memcpy (buf, config->master_key, len + 1);
  else
    *buf = '\0';

  return (strlen (config->master_key));
}

int
horus_set_fattr_client (int fd, struct in_addr *client,
                        u_int32_t start, u_int32_t end)
{
  return horus_ea_config_add_entry (NULL, fd, *client, start, end);
}



int
horus_get_fattr_client (int fd, struct in_addr *client,
                        u_int32_t * start, u_int32_t * end)
{
  int res, i, count, found = 0;
  struct horus_ea_config config;
  struct in_addr ip;

  res =
    _fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE);

  if (res == -1)
    return -errno;
  memcpy (&count, config.entry_count, 4);

  for (i = 0; i < count; i++)
    {

      memcpy (&ip.s_addr, config.entry_table[i].addr, 4);
      if (ip.s_addr == client->s_addr)
        {
          memcpy (start, config.entry_table[i].start_block_num, 4);
          memcpy (end, config.entry_table[i].end_block_num, 4);
          found = 1;
          break;
        }
    }
  if (found)
    return 0;
  else
    return -1;
}

int
horus_get_fattr(int fd, struct horus_ea_config *config)
{

  int res;

  res =
    _fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) config, HORUS_EA_SIZE);

  if (res == -1)
    return -errno;
  else
    return 0;
}



int horus_get_fattr_config_client (struct horus_ea_config *config, struct in_addr *client, u_int32_t *start, u_int32_t *end)
{

  int i, count, found = 0;
  struct in_addr ip;


  memcpy (&count, config->entry_count, 4);
  for (i = 0; i < count; i++)
    {

      memcpy (&ip.s_addr, config->entry_table[i].addr, 4);
      if (ip.s_addr == client->s_addr)
        {
          memcpy (start, config->entry_table[i].start_block_num, 4);
          memcpy (end, config->entry_table[i].end_block_num, 4);
          found = 1;
          break;
        }
    }
  if (found)
    return 0;
  else
    return -1;
}

/* NEW CODE */
int
horus_get_file_config (int fd, struct horus_file_config *config)
{
  int ret;
  ret = _fgetxattr (fd, HORUS_FATTR_NAME, (void *) config,
                    sizeof (struct horus_file_config));
  if (ret < 0)
    log_error ("fgetxattr() failed in %s: %s",
               __FUNCTION__, strerror (errno));
  return ret;
}

int
horus_set_file_config (int fd, struct horus_file_config *config)
{
  int ret;
  ret = _fsetxattr (fd, HORUS_FATTR_NAME, (void *) config,
                    sizeof (struct horus_file_config), 0);
  if (ret < 0)
    log_error ("fsetxattr() failed in %s: %s",
               __FUNCTION__, strerror (errno));
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
      if (IS_HORUS_CLIENT_RANGE_EMPTY(p))
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
horus_clear_client_range (int fd)
{
  int ret;
  int i;
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
    log_error ("fsetxattr() failed in %s: %s",
               __FUNCTION__, strerror (errno));
  return ret;
}
/* End of NEW CODE */

