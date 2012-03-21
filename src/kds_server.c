
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
#include <sys/types.h>

#include <horus_attr.h>
#include <horus_key.h>

#include "kds.h"

static int num_threads = 0;

pthread_mutex_t kds_client_th = PTHREAD_MUTEX_INITIALIZER;

static void
incr_threadcount (void)
{
  pthread_mutex_lock (&kds_client_th);
  num_threads++;
  pthread_mutex_unlock (&kds_client_th);
}

static void
decr_threadcount (void)
{
  pthread_mutex_lock (&kds_client_th);
  num_threads--;
  pthread_mutex_unlock (&kds_client_th);
}

static int
current_threadcount (void)
{

  int val;

  pthread_mutex_lock (&kds_client_th);
  val = num_threads;
  pthread_mutex_unlock (&kds_client_th);
  return val;
}

void *
handle_kds_client (void *p)
{
  int priv_fd, fd, th_val;
  int ret, client_len, rbuf_len;
  short syn_ack = 0x11;
  char *buf;
  struct sockaddr_in client;

  struct key_request kr, kr1;

  pthread_detach (pthread_self ());
  buf = malloc (MAX_RECV_LEN + 1);
  fd = *(int *) p;
  client_len = sizeof (client);

  incr_threadcount ();

  rbuf_len = recvfrom (fd, buf, MAX_RECV_LEN, 0, (struct sockaddr *) &client,
                       &client_len);

  if (rbuf_len <= 0)
    {
      decr_threadcount ();
      pthread_exit (NULL);
    }


  priv_fd = udp_socket ("0.0.0.0", 0);
  if (priv_fd < 0)
    {
      decr_threadcount ();
      pthread_exit (NULL);
    }
  fcntl (priv_fd, F_SETFL, O_NONBLOCK);
  sendto (priv_fd, &syn_ack, sizeof (syn_ack), 0, (struct sockaddr *) &client,
          client_len);

/* Calculate and send key */

  sendto (priv_fd, buf, rbuf_len, 0, (struct sockaddr *) &client, client_len);

  close (priv_fd);
  decr_threadcount ();

  pthread_exit (NULL);
}


int
udp_socket (char *addr, int port)
{
  int sock, on = 1;
  struct sockaddr_in srv;

  memset (&srv, 0, sizeof (srv));
  srv.sin_addr.s_addr = inet_addr (addr);
  srv.sin_port = htons (port);
  srv.sin_family = AF_INET;

  assert ((sock = socket (AF_INET, SOCK_DGRAM, 0)) > 0);
  setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof (on));
  assert (bind (sock, (struct sockaddr *) &srv, sizeof (srv)) >= 0);

  return sock;

}



int
main (int argc, char **argv)
{
  int fd, ret;
  fd_set fds;
  pthread_t th_id[THREAD_MAX];


  fd = server_socket (PF_INET, SOCK_DGRAM,
                      HORUS_KDS_SERVER_PORT, "kds_server_udp");
  assert (fd >= 0);
  FD_ZERO (&fds);
  FD_SET (fd, &fds);
  fcntl (fd, F_SETFL, O_NONBLOCK);

  printf ("KDS started!\n");

  while (1)
    {
      ret = select (fd + 1, &fds, NULL, NULL, NULL);
      assert (ret >= 0);
      if (current_threadcount () >= THREAD_MAX)
        continue;
      pthread_create (&th_id[num_threads], NULL, handle_kds_client,
                      (void *) &fd);
    }

  return 0;
}
