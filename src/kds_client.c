#include <horus.h>

#include <log.h>
#include <shell.h>
#include <shell_history.h>
#include <command.h>
#include <command_shell.h>
#include <thread.h>
#include <network.h>
#include <terminal.h>
#include <openssl.h>

#include <horus_attr.h>
#include <horus_key.h>

#include <kds.h>

int
main (int argc, char *argv[])
{
  int fd;
  int ret, fromlen, slen, rlen;
  struct sockaddr_in srv_addr;
  struct sockaddr_storage from;
  char rbuf[MAX_RECV_LEN];
  char sbuf[MAX_SEND_LEN];
  short ack;
  struct key_request kr, kr1;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s kds_ip \n", argv[0]);
      exit (1);
    }

  memset (&srv_addr, 0, sizeof (srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons (HORUS_KDS_SERVER_PORT);
  inet_pton (AF_INET, argv[1], &srv_addr.sin_addr);

  fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
    {
      fprintf (stderr, "Unable to open socket!\n");
      exit (1);
    }
  /* Just some test code */
  kr.ranges = malloc (sizeof (range) * 2);
  kr.ranges[0].x = kr.ranges[1].y = 123;
  kr.ranges[1].x = kr.ranges[0].y = 456;

  kr.ranges_len = 2;
  kr.filename = strdup ("good morning");
  kr.filename_len = strlen (kr.filename) + 1;

  slen = kr2str (&kr, sbuf, MAX_SEND_LEN);

  rlen = MAX_RECV_LEN;
  client_sendrecv (fd, srv_addr, sbuf, rbuf, slen, &rlen);


}
