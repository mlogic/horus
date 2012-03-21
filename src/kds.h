
#define HORUS_KDS_SERVER_PORT 6666
#define MAX_RECV_LEN 1024
#define MAX_SEND_LEN 1024
#define THREAD_MAX 16

#define KDS_COMMAND_KEYREQ 0x1

struct range {
  int x;
  int y;
};

typedef struct range range;

struct key_request
{
  int buf_len;
  int ranges_len;
  int filename_len;
  range *ranges;
  char *filename;
};


int
client_sendrecv(int fd, struct sockaddr_in srv_addr, char *sbuf, char *rbuf,
                const int slen, int *rlen_p);


/* serializing stuct key_request to a char string */

int
kr2str(struct key_request *kr, char *buf, int buf_size)
{
  int i;
  void *curr_ptr = buf;
  int cmd = KDS_COMMAND_KEYREQ;
  int estimated_size;
  estimated_size = (4 * sizeof(int)) + kr->ranges_len*sizeof(range) 
                   + kr->filename_len;
  if (estimated_size > buf_size)
    return -1;

  memcpy(curr_ptr, &cmd, sizeof(int));
  curr_ptr+= sizeof(int);
  memcpy(curr_ptr, &estimated_size, sizeof(int));
  curr_ptr+= sizeof(int);
  memcpy(curr_ptr, &kr->ranges_len, sizeof(int));
  curr_ptr+= sizeof(int);
  memcpy(curr_ptr, &kr->filename_len, sizeof(int));
  curr_ptr+= sizeof(int);

  for(i=0;i<kr->ranges_len; i++)
  {
    memcpy(curr_ptr, &kr->ranges[i], sizeof(range));
    curr_ptr+= sizeof(range);
  }

  memcpy(curr_ptr, kr->filename, kr->filename_len);
  curr_ptr+=kr->filename_len;
  return ((char *)curr_ptr-buf);
}


int
str2kr(struct key_request *kr, char *buf, int buf_size)
{
  int i;
  void *curr_ptr = buf;
  int cmd, actual_size;

  if (buf_size < 4*sizeof(int)) /* Min size */
    return -1;

  memcpy(&cmd, curr_ptr, sizeof(int));
  assert(cmd == KDS_COMMAND_KEYREQ);
  curr_ptr+= sizeof(int);
  memcpy(&kr->buf_len, curr_ptr, sizeof(int));
  memcpy(&actual_size, curr_ptr, sizeof(int));

  if (actual_size != buf_size)
    return -1;

  curr_ptr+= sizeof(int);
  memcpy(&kr->ranges_len, curr_ptr, sizeof(int));
  curr_ptr+= sizeof(int);
  memcpy(&kr->filename_len, curr_ptr, sizeof(int));
  curr_ptr+= sizeof(int);

  kr->ranges = malloc(sizeof(range) * kr->ranges_len);
  kr->filename = malloc(kr->filename_len);

  for(i=0;i<kr->ranges_len; i++)
  {
    memcpy(&kr->ranges[i], curr_ptr, sizeof(range));
    curr_ptr+= sizeof(range);
  }

  memcpy(kr->filename, curr_ptr, kr->filename_len);
  curr_ptr+=kr->filename_len;
  return ((char *)curr_ptr-buf);
}

