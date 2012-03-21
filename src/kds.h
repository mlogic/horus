
#define HORUS_KDS_SERVER_PORT 6666
#define MAX_RECV_LEN 1024
#define MAX_SEND_LEN 1024
#define THREAD_MAX 16

#define KDS_COMMAND_KEYREQ 0x1

struct range
{
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
client_sendrecv (int fd, struct sockaddr_in srv_addr, char *sbuf, char *rbuf,
                 const int slen, int *rlen_p);


/* serializing stuct key_request to a char string */

int kr2str (struct key_request *kr, char *buf, int buf_size);

int str2kr (struct key_request *kr, char *buf, int buf_size);
