#include <horus.h>
#include <horus_attr.h>
#include <horusio.h>
#include <log.h>
#include <kds_protocol.h>
#include <xts.h>
#include <openssl/md5.h>

//Not required right now
#define TEST_PATH "/home/nakul"

// Configure this to some horus configured file
#define DUMMY_PATH "/scratch/nakul/testfile"

// Change as per your test
#define HORUS_BLOCK_SIZE1 480


int counter=0;

int md5sum_nakul(char *buf, int size)
{
        int n;
        MD5_CTX c;
        int i;
        unsigned char out[MD5_DIGEST_LENGTH];
        MD5_Init(&c);
        for (i=0;i<size;i++)
          MD5_Update(&c, buf+i, 1);

        MD5_Final(out, &c);

        for(n=0; n<MD5_DIGEST_LENGTH; n++)
                fprintf(stderr,"%02x", out[n]);
        fprintf(stderr," ");

        return(0);        
}


ssize_t
horusio_crypt(char *buf,ssize_t size,ssize_t fdpos, int op)
{
  int i, ret, total_blocks,offset,config_fd;
  int block_num, leaf_level=0;
  struct horus_file_config c;
  char *tbuf=NULL,tbuf1=NULL;
  struct aes_xts_cipher *cipher = NULL;
  int key_len = HORUS_KEY_LEN,n;
  struct sockaddr_in serv_addr;
  u_int8_t key[32];
  u_int8_t iv[32];

  memset (key, 0, sizeof(key));
  memset (iv, 0, sizeof(iv));
  tbuf = malloc(size);
  memcpy(tbuf,buf,size);

  md5sum_nakul(tbuf,size);
  memset(buf,0, size);
  config_fd = open(DUMMY_PATH, O_RDONLY);
  ret = horus_get_file_config (config_fd, &c);
  if (ret < 0 || !horus_is_valid_config (&c))
  {
    return -1;
  }
  close(config_fd);
  for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
    if (c.kht_block_size[i])
      leaf_level = i;
  cipher = aes_xts_init ();
  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, HORUS_KDS_SERVER_ADDR, &serv_addr.sin_addr);
  serv_addr.sin_port = htons (6666);
  block_num = fdpos /HORUS_BLOCK_SIZE1;

  total_blocks = size / HORUS_BLOCK_SIZE1;

  for (i=0; i<total_blocks;i++)
  {
    offset = i * HORUS_BLOCK_SIZE1;
    memset (key, 1, sizeof(key));
    client_key_request ((char *)key, &key_len, DUMMY_PATH,
                        leaf_level, block_num+i, &serv_addr);

    if (aes_xts_setkey (cipher, key, 32) != 0)
    {
      printf ("aes_xts_setkey error! %s",strerror(errno));
      abort ();
    }
    memset (iv, 1, sizeof(iv));
 

    if (op == HORUSIO_DECRYPT)
      aes_xts_decrypt (cipher, buf+offset, tbuf+offset,
                       HORUS_BLOCK_SIZE1, iv);
    else
      aes_xts_encrypt (cipher, buf+offset, tbuf+offset,
                       HORUS_BLOCK_SIZE1, iv);

  }
  md5sum_nakul(buf,size);
  fprintf(stderr,"\n");
  counter++;
  if (tbuf)
    free(tbuf);
  if (cipher)
    free(cipher);
  return size;
}


#if 0
ssize_t
horusio_read (int fd, void *buf, size_t size)
{
  int nbyte,ret,block_num,test_path_len;
  int i,leaf_level,config_fd,do_horus = 0, aligned_read = 0;
  off_t fdpos = 0,internal_offset,data_read,actual_read;
  struct stat statbuf;
  char link_path[HORUS_MAX_FILENAME_LEN];
  char real_path[HORUS_MAX_FILENAME_LEN];
  struct horus_file_config c;
  char block_storage[HORUS_BLOCK_SIZE];
  char temp_buf[HORUS_BLOCK_SIZE];
  char *tbuf=NULL;

  u_int8_t key[32];
  u_int8_t iv[32];
  struct aes_xts_cipher *cipher = NULL;
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
          goto exit;
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

    block_num = fdpos /HORUS_BLOCK_SIZE;
    internal_offset = fdpos % HORUS_BLOCK_SIZE;;
    if (internal_offset == 0 && (size%HORUS_BLOCK_SIZE == 0))
      aligned_read = 1;
    if (aligned_read)
    {
      tbuf=malloc(size);
      nbyte = (int) syscall (SYS_read, fd, tbuf, size);
    }
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


      if (aligned_read)
      {
        memset (iv, 0, sizeof(iv));
        aes_xts_decrypt (cipher, buf+data_read, tbuf+data_read,
                         HORUS_BLOCK_SIZE, iv);
      }
      else
      { 
        if (internal_offset == 0 && (size%HORUS_BLOCK_SIZE == 0 ||
            size > HORUS_BLOCK_SIZE))
        {
          //Just decrypt
          nbyte = (int) syscall (SYS_read, fd, temp_buf, HORUS_BLOCK_SIZE);
          if(nbyte == 0)
            goto exit;
          memset (iv, 0, sizeof(iv));

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
          aes_xts_decrypt (cipher, block_storage, temp_buf,
                           HORUS_BLOCK_SIZE, iv);
          memcpy(buf + data_read, block_storage + internal_offset,
                 MIN(HORUS_BLOCK_SIZE - internal_offset, size - data_read));
          actual_read = MIN(HORUS_BLOCK_SIZE - internal_offset, size - data_read);
        }
      }
    }
    if (!aligned_read)
    {
      lseek (fd, fdpos+actual_read, SEEK_SET);
      nbyte=data_read;
    }
  }

exit:
  if (cipher)
    free(cipher);
  return (ssize_t) nbyte;
}

ssize_t
horusio_write (int fd, const void *buf, size_t size)
{
  int nbyte,ret,block_num,do_horus=0,aligned_write=0;
  int i,leaf_level,config_fd,test_path_len;
  off_t fdpos = 0,offset,written,actual_written;
  struct stat statbuf;
  char link_path[HORUS_MAX_FILENAME_LEN];
  char real_path[HORUS_MAX_FILENAME_LEN];
  struct horus_file_config c;
  char block_storage[HORUS_BLOCK_SIZE];
  char temp_buf[HORUS_BLOCK_SIZE];

  u_int8_t key[32];
  u_int8_t iv[32];
  struct aes_xts_cipher *cipher = NULL;
  int key_len = HORUS_KEY_LEN;
  struct sockaddr_in serv_addr;
  char *tbuf = NULL;
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
          goto exit;
      }

    tbuf = malloc(size);
    memset(tbuf, 0, size);
    /* calculate leaf level */
    for (i = 0; i < HORUS_MAX_KHT_DEPTH; i++)
      if (c.kht_block_size[i])
       leaf_level = i;
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, HORUS_KDS_SERVER_ADDR, &serv_addr.sin_addr);
    serv_addr.sin_port = htons (6666);

    actual_written = 0;
    block_num = fdpos /HORUS_BLOCK_SIZE;
    offset = fdpos % HORUS_BLOCK_SIZE;;
    if (offset == 0 && (size%HORUS_BLOCK_SIZE == 0))
      aligned_write = 1;

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
      if (aligned_write)
      {
        memset (iv, 0, sizeof(iv));
        aes_xts_encrypt (cipher, tbuf+written, buf + written,
                         HORUS_BLOCK_SIZE, iv);
        actual_written+=HORUS_BLOCK_SIZE;
      }
      else
      {
//        printf("inside non-aligned!\n");
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
            memset (iv, 0, sizeof(iv));
            aes_xts_decrypt (cipher, block_storage, temp_buf,
                             HORUS_BLOCK_SIZE, iv);
            memcpy(block_storage + offset, buf + written,
                   MIN(HORUS_BLOCK_SIZE - offset, size - written));
          }
          else
            memcpy(block_storage,buf+written,MIN(HORUS_BLOCK_SIZE - offset, size - written));
          memset (iv, 0, sizeof(iv));
          aes_xts_encrypt (cipher, temp_buf, block_storage,
                           HORUS_BLOCK_SIZE, iv);
          memset (iv, 0, sizeof(iv));        
          lseek (fd, fdpos-offset+written, SEEK_SET);
          nbyte = syscall (SYS_write, fd, temp_buf, HORUS_BLOCK_SIZE);
          actual_written = MIN(HORUS_BLOCK_SIZE - offset, size - written);
        }
      }
    }
//    printf("setting lseek to %d\n", fdpos+written);
    if (aligned_write)
    {
      nbyte = syscall (SYS_write, fd, tbuf, size);
    }
    else
    {
      lseek (fd, fdpos+actual_written, SEEK_SET);
      nbyte=written;
    }
  }

exit:
  if (cipher)
    free(cipher);
  if (tbuf)
    free(tbuf);
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


#endif
