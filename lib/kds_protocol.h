
#ifndef _KDS_PROTOCOL_H_
#define _KDS_PROTOCOL_H_

#define HORUS_KDS_SERVER_ADDR   "127.0.0.1"
#define HORUS_KDS_SERVER_PORT   6666

#define HORUS_THREAD_MAX        16
#define HORUS_MAX_KHT_DEPTH     16
#define HORUS_MAX_KEY_LEN       41
#define HORUS_MAX_FILENAME_LEN  256

struct key_request_packet
{
  u_int32_t x;
  u_int32_t y;
  char filename[HORUS_MAX_FILENAME_LEN];
};
typedef struct key_request_packet key_request_packet;

/* align things... */
struct key_response_packet
{
  u_int32_t x;
  u_int32_t y;
  char filename[HORUS_MAX_FILENAME_LEN];
  u_int32_t err;
  u_int32_t kht_block_size[HORUS_MAX_KHT_DEPTH];
  u_int32_t key_len;
  char key[HORUS_MAX_KEY_LEN];
};
typedef struct key_response_packet key_response_packet;

int
client_sendrecv (int fd, struct sockaddr_in *srv_addr,
                 const key_request_packet *kreq,
                 key_response_packet *kresp);

#endif /*_KDS_PROTOCOL_H_*/

