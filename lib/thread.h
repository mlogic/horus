
#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

struct thread {
  struct thread_master *master;
  pthread_t pthread;
  int index;
  int state;
  void *(*func) (void *thread);
  char *name;
  int readfd;
  int writefd;
  void *arg;
  void *ret;
};

#define THREAD_STATE_NONE     0
#define THREAD_STATE_RELEASED 1
#define THREAD_STATE_RUNNING  2
#define THREAD_STATE_ENDING   3
#define THREAD_STATE_MAX      4

struct thread_master
{
  int nthreads;
  struct thread *threads;

#define COND_THREAD 0
#define COND_MAX 1
  pthread_mutex_t cond_mutex[COND_MAX];
  pthread_cond_t cond[COND_MAX];
};

struct thread_master *thread_master_create ();
void thread_master_delete (struct thread_master *m);

struct thread *thread_get (struct thread_master *m);

int thread_add_read (struct thread *t);
void thread_release (struct thread *t);

int thread_running (struct thread_master *m);
int thread_run (struct thread_master *m);

#endif /*_THREAD_H_*/

