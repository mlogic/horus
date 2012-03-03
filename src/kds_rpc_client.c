#include <stdio.h>
#include <rpc/rpc.h>
#include "kds_rpc.h"

int
main (int argc, char *argv[])
{
  CLIENT *cl;
  char *filename, *server;
  int xy[MAXKEYS][2],num_keys = 0,i,j;
  struct key_request req;
  struct key_rtn *resp;
  

  if (argc < 5)
    {
      fprintf (stderr, "usage %s server filename x y [x y ...]\n", argv[0]);
      exit (1);
    }
  if (argc%2==0)
  {
    fprintf (stderr, "Incorrect number of arguments!\n");
    exit(1);
  }
 
  num_keys = (argc - 3)/2;
  for (i=0,j=3;i<num_keys;i++,j=j+2)
  {
    xy[i][0] = atoi(argv[j]);
    xy[i][1] = atoi(argv[j+1]);
  }

  if (num_keys > MAXKEYS)
  {
    fprintf(stderr, "Max num_of_keys allowed is %d\n", MAXKEYS);
    exit(1);
  }
  server = argv[1];
  filename = argv[2];
  cl = clnt_create (server, KDS_RPC, KEYREQVERS, "udp");
  if (cl == NULL)
    {
      clnt_pcreateerror (server);
      exit (1);
    }

  req.filename = filename;
  req.ranges.ranges_len = num_keys;
  req.ranges.ranges_val = calloc(sizeof(struct range), num_keys);
  for (i=0;i<num_keys;i++)
  {
    req.ranges.ranges_val[i].x = xy[i][0];
    req.ranges.ranges_val[i].y =  xy[i][1];
  }
  resp = keyreq_1 (&req, cl);
  if (resp == NULL)
    {
      clnt_perror (cl, "call failed:");
      exit(1);
    }
  clnt_destroy (cl);
  if (resp->err == 0)
  {
    for (i=0;i<resp->keys.keys_len;i++)
    {
      if (resp->keys.keys_val[i].err == 0)
        printf ("key %d,%d = %s\n", xy[i][0], xy[i][1], resp->keys.keys_val[i].key);
      else
        printf ("key %d,%d err = %s\n", xy[i][0], xy[i][1], strerror(resp->keys.keys_val[i].err));
    }
  }
  else
    printf ("err = %d\n", resp->err);
}