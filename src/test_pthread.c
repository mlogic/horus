
/*
 * If run by ./a.out 4 0 (no sleep), it is likely to be deadlock.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>

#define MUTEX_MAX 1
pthread_mutex_t mutex[MUTEX_MAX];

#define COND_START 0
#define COND_MAX 1
pthread_mutex_t cond_mutex[COND_MAX];
pthread_cond_t cond[COND_MAX];

#define THREAD_MAX 256
struct thread_arg {
  pthread_t pthread;
  int index;
  void *ret;
  void * (*func) (void *arg);
};
struct thread_arg arg[THREAD_MAX];

void *
test_func (void *thread_arg)
{
  struct thread_arg *arg = (struct thread_arg *) thread_arg;
  printf ("thread[%d]: 1. started.\n", arg->index);

  printf ("thread[%d]: 2. wait START condition.\n", arg->index);
  pthread_mutex_lock (&cond_mutex[COND_START]);
  pthread_cond_wait (&cond[COND_START], &cond_mutex[COND_START]);
  pthread_mutex_unlock (&cond_mutex[COND_START]);

  printf ("thread[%d]: 3. ending.\n", arg->index);
  return thread_arg;
}

int
main (int argc, char **argv)
{
  int i;
  unsigned int nthreads = 0;
  unsigned int seconds = 1;

  if (argc > 1)
    nthreads = atoi (argv[1]);
  if (nthreads < 1 || THREAD_MAX < nthreads)
    nthreads = THREAD_MAX;

  if (argc > 2)
    seconds = atoi (argv[2]);

  printf ("main: nthreads: %d, sleep seconds: %d.\n", nthreads, seconds);

  /* Pthread initialization */
  for (i = 0; i < MUTEX_MAX; i++)
    pthread_mutex_init (&mutex[i], NULL);
  for (i = 0; i < COND_MAX; i++)
    {
      pthread_mutex_init (&cond_mutex[i], NULL);
      pthread_cond_init (&cond[i], NULL);
    }
  memset (arg, 0, sizeof (arg));
  for (i = 0; i < THREAD_MAX; i++)
    {
      arg[i].index = i;
      arg[i].func = test_func;
    }

  for (i = 0; i < nthreads; i++)
    {
      pthread_create (&arg[i].pthread, NULL, arg[i].func, &arg[i]);
    }

  if (seconds != 0)
    sleep (seconds);

  printf ("main: broadcast START.\n");
  pthread_cond_broadcast (&cond[COND_START]);

  for (i = 0; i < nthreads; i++)
    {
      printf ("main: wait thread %d.\n", i);
      pthread_join (arg[i].pthread, &arg[i].ret);
      printf ("main: wait thread %d: returned.\n", i);
    }

  return 0;
}



