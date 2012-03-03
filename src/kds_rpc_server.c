#include <horus.h>
#include <openssl.h>
#include <horus_attr.h>
#include <horus_key.h>

#include <rpc/rpc.h>
#include "kds_rpc.h"

struct key_rtn *
keyreq_1_svc (struct key_request *input, struct svc_req *req)
{

  int fd = -1, i;
  uint32_t start_block, end_block, x, y, range_size, blk, start, end;
  int ret, key_len, parent_len,ranges_len;
  char key[SHA_DIGEST_LENGTH], range[30], parent[30];
  char master[30];
  char *filename;
  struct sockaddr_in *client_addr;

  static struct key_rtn key_out;

  xdr_free (xdr_key_rtn, &key_out);

  key_out.err = -1;

  filename = input->filename;
  ranges_len = input->ranges.ranges_len;
  client_addr = svc_getcaller (req->rq_xprt);

  fprintf (stderr, "Got request for %s num of keys %d\n", filename, ranges_len);

  key_out.keys.keys_len=0;
  key_out.err = 0;
  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "Unable to open %s!\n", filename);
      key_out.err = EIO;
      goto exit;
    }
  else
    {
      ret = horus_get_fattr_client (fd, &(client_addr->sin_addr),
                                    &start_block, &end_block);
      if (ret != 0)
        {
          fprintf (stderr, "Range for %s not configured for client\n",
                   filename);
          key_out.err = EINVAL;
          goto exit;
        }


      ret = horus_get_fattr_masterkey (fd, master, 30);
      if (ret < 0)
        {
          fprintf (stderr, "Unable to read master-key ret = %d!\n", ret);
          key_out.err = EINVAL;
          goto exit;
        }
      master[ret + 1] = '\0';   /* ret actually has length of master-key */


      key_out.keys.keys_len = ranges_len;
      key_out.keys.keys_val = calloc(sizeof(struct rangekey), ranges_len);

      for (i = 0; i < ranges_len; i++)
      {
        /* Convert x,y into start-block  and end-block  number */
        x = input->ranges.ranges_val[i].x;
        y = input->ranges.ranges_val[i].y;

 
        start = (y * block_size[x] / MIN_CHUNK_SIZE);
        end = ((y + 1) * block_size[x] / MIN_CHUNK_SIZE);
        if ((start >= start_block) && (end <= end_block))
        {
          snprintf (parent, sizeof (parent), "%s", master);
          parent_len = strlen (master);
          ret = horus_key_by_master (key, &key_len, x, y, parent, parent_len);
          if (ret == 0)
          {
            key_out.keys.keys_val[i].key = strdup (print_key (key, key_len));
            fprintf(stderr, "%d %d %s\n",  x, y, key_out.keys.keys_val[i].key);
            key_out.keys.keys_val[i].err = 0;
          }
          else
          {
            fprintf (stderr, "Error: horus_key_by_master error %d\n", ret);
            key_out.keys.keys_val[i].key =  strdup ("");
            key_out.keys.keys_val[i].err = ret;
          }
        }
      else
        {
          fprintf (stderr, "Access denied!\n");
          key_out.keys.keys_val[i].err  = EACCES;
          key_out.keys.keys_val[i].key =  strdup ("");
        }
      }
    }
exit:
  if (fd > 0)
    close (fd);
  return &key_out;
}