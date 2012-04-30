#include <horus.h>
#include <horus_attr.h>

#include <log.h>
#include <kds_protocol.h>
int
horusio_open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int fd = -1;

  if (oflag & O_CREAT)
    {
      va_list ap;
      va_start (ap, oflag);
      mode = va_arg (ap, int);
      va_end (ap);
      fd = (int) syscall (SYS_open, path, oflag, mode);
    }
  else
    {
      fd = (int) syscall (SYS_open, path, oflag);
    }

  log_open (fd, path, oflag, mode);
  
#ifdef ENABLE_HORUS
  horus_open (fd, path, oflag, mode);
#endif /*ENABLE_HORUS*/

  return fd;
}

#ifdef SYS_socket
int
horusio_socket (int domain, int type, int protocol)
{
  int fd = -1;

  fd = (int) syscall (SYS_socket, domain, type, protocol);
  log_socket (fd, domain, type, protocol);

  return fd;
}
#endif /*SYS_socket*/

ssize_t
horusio_read (int fd, void *buf, size_t size)
{
  int nbyte;
  off_t fdpos = 0;
  struct stat statbuf;

  fstat (fd, &statbuf);

  if (S_ISREG (statbuf.st_mode))
    fdpos = lseek (fd, 0, SEEK_CUR);

  nbyte = (int) syscall (SYS_read, fd, buf, size);

  if (S_ISREG (statbuf.st_mode))
    log_read (fd, fdpos, buf, (size_t) nbyte, size);

#ifdef ENABLE_HORUS
  /* Horus function only for regular file and successful read. */
  if (S_ISREG (statbuf.st_mode) && nbyte > 0)
    horus_pread (fd, buf, (size_t) nbyte, fdpos);
#endif /*ENABLE_HORUS*/

  return (ssize_t) nbyte;
}

ssize_t
horusio_write (int fd, const void *buf, size_t size)
{
  int nbyte,ret,block_num;
  int i,leaf_level,config_fd;
  off_t fdpos = 0,offset,written,actual_written;
  struct stat statbuf;
  char link_path[HORUS_MAX_FILENAME_LEN];
  char real_path[HORUS_MAX_FILENAME_LEN];
  struct horus_file_config c;
  char block_data[HORUS_BLOCK_SIZE];
  char block_storage[HORUS_BLOCK_SIZE];
  char temp_buf[HORUS_BLOCK_SIZE];
  char *block;

  u_int8_t key[HORUS_KEY_LEN];
  u_int8_t iv[HORUS_KEY_LEN];
  struct aes_xts_cipher *cipher;
  int key_len = HORUS_KEY_LEN;
  struct sockaddr_in serv_addr;
  memset (key, 0, sizeof(key));
  memset (iv, 0, sizeof(iv));

  fstat (fd, &statbuf);

  if (S_ISREG (statbuf.st_mode))
  {
    fdpos = lseek (fd, 0, SEEK_CUR);
    sprintf(link_path, "/proc/self/fd/%d", fd);
/* BTIO test seems to delete old file, which breaks our system of
 * using EA. As an alternative, we request keys from a different file */

//    ret = readlink(link_path, real_path, HORUS_MAX_FILENAME_LEN);
//    assert(ret>0);
//    real_path[ret+1] = 0;
    sprintf(real_path, "/home/nakul/horus/testfile");  

    config_fd = open(real_path, O_RDONLY);
    ret = horus_get_file_config (config_fd, &c);
    if (ret < 0 || !horus_is_valid_config (&c))
      {
          nbyte = (int) syscall (SYS_write, fd, buf, size);
          goto exit;
      }

    /* calculate leaf level */
    for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
      if (c.kht_block_size[i])
        leaf_level = i;
    cipher = aes_xts_init ();
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, HORUS_KDS_SERVER_ADDR, &serv_addr.sin_addr);
    serv_addr.sin_port = htons (6666);

    for (written = 0; written != size; written += actual_written)
    {
      block_num = (fdpos + written)/HORUS_BLOCK_SIZE;
      offset = (fdpos + written )% HORUS_BLOCK_SIZE;
      client_key_request ((char *)key, &key_len, real_path,
                          leaf_level, block_num, &serv_addr);
      aes_xts_setkey (cipher, key, HORUS_KEY_LEN);

      if (offset == 0 && (size%HORUS_BLOCK_SIZE == 0 ||
          size > HORUS_BLOCK_SIZE))
      {
        //Just encrypt
        /* Use block id as IV */
        *(unsigned long *)iv = block_num;
        aes_xts_encrypt (cipher, block_storage, buf + written,
                         HORUS_BLOCK_SIZE, iv); 
        actual_written = (int) syscall (SYS_write, fd, block_storage, HORUS_BLOCK_SIZE);

      }
      else
      {
        //read one block
        lseek (fd, fdpos-offset, SEEK_SET);
        memset(temp_buf, 0, HORUS_BLOCK_SIZE);
        nbyte = (int) syscall (SYS_read, fd, temp_buf, HORUS_BLOCK_SIZE);
        //decrypt
        *(unsigned long *)iv = block_num;
        aes_xts_decrypt (cipher, block_storage, buf + written,
                         HORUS_BLOCK_SIZE, iv);
        memcpy(temp_buf + offset, buf + written,
               MIN(HORUS_BLOCK_SIZE - offset, size - written));
        aes_xts_encrypt (cipher, block_storage, temp_buf,
                         HORUS_BLOCK_SIZE, iv);
        syscall (SYS_write, fd, buf, size);
        actual_written = MIN(HORUS_BLOCK_SIZE - offset, size - written);
 
        //insert new data
        //encrypt
      }
      lseek (fd, fdpos+written, SEEK_SET);
      nbyte=written;
    }

  }

exit:
  if (S_ISREG (statbuf.st_mode))
    log_write (fd, fdpos, (void *) buf, nbyte, size);

  return (ssize_t) nbyte;
}

int
horusio_close (int fd)  
{
  struct stat statbuf;

  fstat (fd, &statbuf);

#ifdef ENABLE_HORUS
  horus_close (fd);
#endif /*ENABLE_HORUS*/

  if (S_ISREG (statbuf.st_mode))
    log_close (fd);

  return (int) syscall (SYS_close, fd);
}

int
horusio_unlink (const char *path)
{
  log_unlink (path);

  return syscall (SYS_unlink, path);
}

int
horusio_dup2 (int fd, int fd2)
{
  struct stat statbuf;

  fstat (fd, &statbuf);

#ifdef ENABLE_HORUS
  horus_close (fd2);
#endif /*ENABLE_HORUS*/

  if (S_ISREG (statbuf.st_mode))
    log_dup2 (fd, fd2);

  return syscall (SYS_dup2, fd, fd2);
}


