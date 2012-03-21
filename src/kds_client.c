/*
  Part taken from tftp.c and tftpd.c

  Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
  2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software
  Foundation, Inc.

  This file is part of GNU Inetutils.

  GNU Inetutils is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  GNU Inetutils is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see `http://www.gnu.org/licenses/'. */

/*
 * Copyright (c) 1983, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */



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

static void
set_port (struct sockaddr_storage *ss, in_port_t port)
{
  switch (ss->ss_family)
    {
    case AF_INET6:
      ((struct sockaddr_in6 *) ss)->sin6_port = htons(port);
      break;
    case AF_INET:
    default:
      ((struct sockaddr_in *) ss)->sin_port = htons(port);
      break;
    }
}


static in_port_t
get_port (struct sockaddr_storage *ss)
{
  switch (ss->ss_family)
    {
    case AF_INET6:
      return ntohs (((struct sockaddr_in6 *) ss)->sin6_port);
      break;
    case AF_INET:
    default:
      return ntohs (((struct sockaddr_in *) ss)->sin_port);
      break;
    }
}

int
client_sendrecv(int fd, struct sockaddr_in srv_addr, char *sbuf, char *rbuf,
                const int slen, int *rlen_p)
{
  int ret = -1,fromlen, rlen;
  short ack;
  struct sockaddr_storage from;

  assert(rlen_p !=NULL);
  rlen = *rlen_p;
  
  ret = sendto(fd, sbuf, slen, 0,
               (struct sockaddr *) &srv_addr, sizeof(srv_addr));
  assert (slen == ret);

  do
  {
    fromlen = sizeof(from);
    ret = recvfrom(fd, &ack, sizeof(ack), 0, (struct sockaddr *) &from,
                   &fromlen);
  }while(ret <= 0);

  set_port((struct sockaddr_storage *)&srv_addr, get_port(&from));

  do
  {
    fromlen = sizeof(from);
    ret = recvfrom(fd, rbuf, rlen, 0, (struct sockaddr *) &from, &fromlen);
  }while(ret <= 0);

  *rlen_p = ret;
  return 0;
}

int main(int argc, char *argv[])
{
  int fd;
  int ret, fromlen,slen,rlen;
  struct sockaddr_in srv_addr;
  struct sockaddr_storage from;
  char rbuf[MAX_RECV_LEN];
  char sbuf[MAX_SEND_LEN];
  short ack;

  struct key_request kr,kr1;

  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s kds_ip \n", argv[0]);
    exit(1);
  }

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(HORUS_KDS_SERVER_PORT);
  inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
  {
    fprintf(stderr, "Unable to open socket!\n");
    exit(1);
  }
  /* Just some test code */
  kr.ranges=malloc(sizeof(range)*2);
  kr.ranges[0].x = kr.ranges[1].y = 123;
  kr.ranges[1].x = kr.ranges[0].y = 456;

  kr.ranges_len = 2;
  kr.filename = strdup("good morning");
  kr.filename_len = strlen(kr.filename) + 1;
  
  kr2str(&kr, sbuf, MAX_SEND_LEN);

  slen = MAX_SEND_LEN;
  rlen = MAX_RECV_LEN;
  client_sendrecv(fd, srv_addr, sbuf, rbuf, slen, &rlen);


}
