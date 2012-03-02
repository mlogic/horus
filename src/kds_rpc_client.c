#include <stdio.h>
#include <rpc/rpc.h>
#include "kds_rpc.h"

int main(int argc, char *argv[])
{
  CLIENT *cl;
  char *filename,*server;
  int x,y;
  struct key_request req;
  struct key_rtn *resp;

  if (argc < 5)
  {
    fprintf(stderr, "usage %s server filename x y\n", argv[0]);
    exit(1);
  }

  server = argv[1];
  filename = argv[2];
  x = atoi(argv[3]);
  y = atoi(argv[4]);
  cl = clnt_create(server, KDS_RPC, KEYREQVERS, "udp");
  if (cl == NULL)
  {
    clnt_pcreateerror(server);
    exit(1);
  }

  req.filename=filename;
  req.x = x;
  req.y = y;
  resp = keyreq_1(&req, cl);
  if (resp == NULL)
  {
    clnt_perror(cl, "call failed:");
  }
  clnt_destroy(cl);
    if (resp->err == 0)
    	printf("key = %s\n", resp->key);
    else
        printf("err = %d\n", resp->err);
}
   
