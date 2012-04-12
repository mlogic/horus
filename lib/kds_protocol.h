
#ifndef _KDS_PROTOCOL_H_
#define _KDS_PROTOCOL_H_

#define HORUS_KDS_SERVER_ADDR   "127.0.0.1"
#define HORUS_KDS_SERVER_PORT   6666

#define HORUS_THREAD_MAX        64
#define HORUS_MAX_KHT_DEPTH     16
#define HORUS_MAX_KEY_LEN       41
#define HORUS_MAX_FILENAME_LEN  256

#define HORUS_ERR_NONE              0
#define HORUS_ERR_OPEN              1
#define HORUS_ERR_CONFIG_GET        2
#define HORUS_ERR_CONFIG_SET        3
#define HORUS_ERR_NO_SUCH_CLIENT    4
#define HORUS_ERR_REQ_OUT_OF_RANGE  5
#define HORUS_ERR_REQ_NOT_ALLOWED   6
#define HORUS_ERR_UNKNOWN           7
#define HORUS_ERR_MAX               8

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
  u_int16_t err;
  u_int16_t suberr;
  u_int32_t kht_block_size[HORUS_MAX_KHT_DEPTH];
  u_int32_t key_len;
  char key[HORUS_MAX_KEY_LEN];
};
typedef struct key_response_packet key_response_packet;

char *
horus_strerror (u_int16_t err);

#endif /*_KDS_PROTOCOL_H_*/

