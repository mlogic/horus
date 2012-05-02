#include <horus.h>
#include <horus_attr.h>

#include <log.h>
#include <kds_protocol.h>

#define TEST_PATH "a/home/nakul/horus"
int
horusio_open (const char *path, int oflag, ...)
{
  mode_t mode = 0;
  int fd = -1;

  oflag = oflag & (~O_RDONLY);
  oflag = oflag & (~O_WRONLY);
  oflag = oflag | O_RDWR;
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
  int nbyte,ret,block_num,test_path_len;
  int i,leaf_level,config_fd,do_horus = 0;
  off_t fdpos = 0,internal_offset,data_read,actual_read;
  struct stat statbuf;
  char link_path[HORUS_MAX_FILENAME_LEN];
  char real_path[HORUS_MAX_FILENAME_LEN];
  struct horus_file_config c;
  char block_data[HORUS_BLOCK_SIZE];
  char block_storage[HORUS_BLOCK_SIZE];
  char temp_buf[HORUS_BLOCK_SIZE];
  char *block;
  unsigned char *p;
  

  u_int8_t key[32];
  u_int8_t iv[32];
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

    ret = readlink(link_path, real_path, HORUS_MAX_FILENAME_LEN);
    assert(ret>0);
    real_path[ret+1] = 0;
    test_path_len = strlen(TEST_PATH);
    if (strncmp(TEST_PATH,real_path,test_path_len)==0)
    {
      do_horus = 1;
    }
    else
      do_horus = 0;

    memset(real_path, 0,HORUS_MAX_FILENAME_LEN);

    sprintf(real_path, "/home/nakul/horus/testfile");

    config_fd = open(real_path, O_RDONLY);
    ret = horus_get_file_config (config_fd, &c);
    if (ret < 0 || !horus_is_valid_config (&c) || do_horus == 0)
      {
          nbyte = (int) syscall (SYS_read, fd, buf, size);
          goto exit1;
      }
    close(config_fd);
    /* calculate leaf level */
    for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
      if (c.kht_block_size[i])
        leaf_level = i;
    cipher = aes_xts_init ();
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, HORUS_KDS_SERVER_ADDR, &serv_addr.sin_addr);
    serv_addr.sin_port = htons (6666);

    for (data_read = 0; data_read < size; data_read += actual_read)
    {
      block_num = (fdpos + data_read)/HORUS_BLOCK_SIZE;
      internal_offset = (fdpos + data_read )% HORUS_BLOCK_SIZE;
      client_key_request ((char *)key, &key_len, real_path,
                          leaf_level, block_num, &serv_addr);
      if (aes_xts_setkey (cipher, key, 32) != 0)
      {
        printf ("aes_xts_setkey error! %s",strerror(errno));
        abort ();
      }


      if (internal_offset == 0 && (size%HORUS_BLOCK_SIZE == 0 ||
          size > HORUS_BLOCK_SIZE))
      {
        //Just decrypt
        nbyte = (int) syscall (SYS_read, fd, temp_buf, HORUS_BLOCK_SIZE);
        if(nbyte == 0)
          goto exit;
        /* Use block id as IV */
        memset (iv, 0, sizeof(iv));
//        *(unsigned long *)iv = block_num+1;

        aes_xts_decrypt (cipher, block_storage, temp_buf,
                         HORUS_BLOCK_SIZE, iv);
        memcpy(buf+data_read, block_storage, HORUS_BLOCK_SIZE);
        actual_read = HORUS_BLOCK_SIZE;

      }
      else
      {
        //read one block
        lseek (fd, fdpos+data_read-internal_offset, SEEK_SET);
        memset(temp_buf, 0, HORUS_BLOCK_SIZE);
        nbyte = (int) syscall (SYS_read, fd, temp_buf, HORUS_BLOCK_SIZE);
        if (nbyte == 0)
          goto exit;
        //decrypt
        memset (iv, 0, sizeof(iv));
//        *(unsigned long *)iv = block_num+1;
        aes_xts_decrypt (cipher, block_storage, temp_buf,
                         HORUS_BLOCK_SIZE, iv);
        memcpy(buf + data_read, block_storage + internal_offset,
               MIN(HORUS_BLOCK_SIZE - internal_offset, size - data_read));
        actual_read = MIN(HORUS_BLOCK_SIZE - internal_offset, size - data_read);

      }
    }
    lseek (fd, fdpos+actual_read, SEEK_SET);
    nbyte=data_read;
  }

  p = buf;
//  printf("read end: fdpos = %u size = %u buf[0] = %u temp_buf[0] = %u nbyte = %d\n", fdpos, size, p[0], (unsigned char)temp_buf[0], nbyte);

exit:
  if (cipher>0)
    free(cipher);
exit1:
  return (ssize_t) nbyte;
}

ssize_t
horusio_write (int fd, const void *buf, size_t size)
{
  int nbyte,ret,block_num,do_horus=0;
  int i,leaf_level,config_fd,test_path_len;
  off_t fdpos = 0,offset,written,actual_written;
  struct stat statbuf;
  char link_path[HORUS_MAX_FILENAME_LEN];
  char real_path[HORUS_MAX_FILENAME_LEN];
  struct horus_file_config c;
  char block_data[HORUS_BLOCK_SIZE];
  char block_storage[HORUS_BLOCK_SIZE];
  char temp_buf[HORUS_BLOCK_SIZE];
  char *block;
  unsigned char *p;

  u_int8_t key[32];
  u_int8_t iv[32];
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
//    printf("fdpos = %d\n", fdpos);
/* BTIO test seems to delete old file, which breaks our system of
 * using EA. As an alternative, we request keys from a different file */

    ret = readlink(link_path, real_path, HORUS_MAX_FILENAME_LEN);
    assert(ret>0);
    real_path[ret+1] = 0;
    test_path_len = strlen(TEST_PATH);
    if (strncmp(TEST_PATH,real_path,test_path_len)==0)
    {
      do_horus = 1;
    }
    else
      do_horus = 0;
    memset(real_path, 0,HORUS_MAX_FILENAME_LEN);
    sprintf(real_path, "/home/nakul/horus/testfile");

    config_fd = open(real_path, O_RDONLY);
    ret = horus_get_file_config (config_fd, &c);
    close(config_fd);
    if (ret < 0 || !horus_is_valid_config (&c) || do_horus == 0)
      {
          nbyte = (int) syscall (SYS_write, fd, buf, size);
          goto exit1;
      }

    /* calculate leaf level */
    for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
      if (c.kht_block_size[i])
       leaf_level = i;
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, HORUS_KDS_SERVER_ADDR, &serv_addr.sin_addr);
    serv_addr.sin_port = htons (6666);

    p = buf;
//    printf("write: fdpos = %u, size = %u buf[0] = %u\n", fdpos, size, p[0]);
    actual_written = 0;
    for (written = 0; written < size; written += actual_written)
    {
      block_num = (fdpos + written)/HORUS_BLOCK_SIZE;
      offset = (fdpos + written )% HORUS_BLOCK_SIZE;
      memset (key, 0, sizeof(key));
      client_key_request ((char *)key, &key_len, real_path,
                          leaf_level, block_num, &serv_addr);

      cipher = aes_xts_init();
      ret = aes_xts_setkey (cipher, key, 32);
      if (ret != 0)
      {
        printf ("aes_xts_setkey err %d\n",ret);
        abort();
      }
      if (offset == 0 && (size%HORUS_BLOCK_SIZE == 0 ||
          size > HORUS_BLOCK_SIZE))
      {
        //Just encrypt
        /* Use block id as IV */
//        *(unsigned long *)iv = block_num+1;
        memset (iv, 0, sizeof(iv));
        aes_xts_encrypt (cipher, block_storage, buf + written,
                         HORUS_BLOCK_SIZE, iv); 
        actual_written = nbyte = (int) syscall (SYS_write, fd, block_storage, HORUS_BLOCK_SIZE);

      }
      else
      {
        lseek (fd, fdpos-offset+written, SEEK_SET);
        memset(temp_buf, 0, HORUS_BLOCK_SIZE);
        nbyte = (int) syscall (SYS_read, fd, temp_buf, HORUS_BLOCK_SIZE);
        if (nbyte)
        {
//          *(unsigned long *)iv = block_num+1;
          memset (iv, 0, sizeof(iv));
          aes_xts_decrypt (cipher, block_storage, temp_buf,
                           HORUS_BLOCK_SIZE, iv);
//          printf("block_storage _ decrypted = %s\n",block_storage);
          memcpy(block_storage + offset, buf + written,
                 MIN(HORUS_BLOCK_SIZE - offset, size - written));
        }
        else
          memcpy(block_storage,buf+written,MIN(HORUS_BLOCK_SIZE - offset, size - written));
//        printf("block_storage _ new data = %s\n",block_storage);
//        *(unsigned long *)iv = block_num+1;
        memset (iv, 0, sizeof(iv));
        aes_xts_encrypt (cipher, temp_buf, block_storage,
                         HORUS_BLOCK_SIZE, iv);
//        *(unsigned long *)iv = block_num+1;
        memset (iv, 0, sizeof(iv));        
        memset(block_storage, 0, HORUS_BLOCK_SIZE);
        lseek (fd, fdpos-offset+written, SEEK_SET);
        nbyte = syscall (SYS_write, fd, temp_buf, HORUS_BLOCK_SIZE);
        actual_written = MIN(HORUS_BLOCK_SIZE - offset, size - written);
 
      }
    }
//    printf("setting lseek to %d\n", fdpos+written);
    lseek (fd, fdpos+actual_written, SEEK_SET);
    nbyte=written;
  }

exit:
  if (cipher >0)
    free(cipher);
exit1:
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


