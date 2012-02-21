/* Copyright Nakul Dhotre UCSC */


#include <horus_attr.h>


int
horus_ea_config_init (struct horus_ea_config *config)
{
  bzero (config, sizeof (struct horus_ea_config));
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
    fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);
  if (res == -1)
    {
      horus_ea_config_init (&config);
    }

  strncpy (config.master_key, key, HORUS_MASTERKEY_LEN);

set_attr:
  res =
    fsetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);
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
    fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);

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



set_attr:
  res =
    fsetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);
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
    lgetxattr (path, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE,
               0);

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
    fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);

  if (res == -1)
    return -errno;

  len = strlen (config.master_key);
  if (len + 1 > bufsiz)
    return -ENOSPC;

  if (len > 0)
    memcpy (buf, config.master_key, len + 1);
  else
    *buf = '\0';

  return (strlen (config.master_key));
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
  int res, len, i, count, found = 0;
  struct horus_ea_config config;
  struct in_addr ip;

  res =
    fgetxattr (fd, HORUS_EA_NAME, (unsigned char *) &config, HORUS_EA_SIZE, 0);

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