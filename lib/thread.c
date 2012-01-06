
#include <thread.h>

struct thread_master *
thread_master_create (int nthreads)
{
  struct thread_master *m;
  int i;

  m = (struct thread_master *)
    malloc (sizeof (struct thread_master));
  assert (m);
  memset (m, 0, sizeof (struct thread_master));

  m->nthreads = nthreads;

  m->threads = (struct thread *)
    malloc (nthreads * sizeof (struct thread));
  assert (m->threads);
  memset (m->threads, 0, sizeof (nthreads * sizeof (struct thread)));

  for (i = 0; i < m->nthreads; i++)
    {
      m->threads[i].master = m;
      m->threads[i].index = i;
      m->threads[i].state = THREAD_STATE_RELEASED;
    }

  for (i = 0; i < COND_MAX; i++)
    {
      pthread_mutex_init (&m->cond_mutex[i], NULL);
      pthread_cond_init (&m->cond[i], NULL);
    }

  return m;
}

void
thread_master_delete (struct thread_master *m)
{
  int i;

  for (i = 0; i < COND_MAX; i++)
    {
      pthread_mutex_destroy (&m->cond_mutex[i]);
      pthread_cond_destroy (&m->cond[i]);
    }

  free (m->threads);
  free (m);
}

struct thread *
thread_get (struct thread_master *m)
{
  int i;
  for (i = 0; i < m->nthreads; i++)
    {
      if (m->threads[i].state == THREAD_STATE_RELEASED)
        break;
    }

  if (i == m->nthreads)
    return NULL;

  return &m->threads[i];
}

int
thread_add_read (struct thread *t)
{
  t->state = THREAD_STATE_RUNNING;
  pthread_create (&t->pthread, NULL, t->func, t);
  return 0;
}

void
thread_release (struct thread *t)
{
  t->state = THREAD_STATE_ENDING;
  pthread_cond_broadcast (&t->master->cond[COND_THREAD]);
}

int
thread_running (struct thread_master *master)
{
  return 1;
}

int
thread_run (struct thread_master *m)
{
  int i;

  pthread_mutex_lock (&m->cond_mutex[COND_THREAD]);
  pthread_cond_wait (&m->cond[COND_THREAD], &m->cond_mutex[COND_THREAD]);
  pthread_mutex_unlock (&m->cond_mutex[COND_THREAD]);

  for (i = 0; i < m->nthreads; i++)
    {
      if (m->threads[i].state == THREAD_STATE_ENDING)
        {
          pthread_join (m->threads[i].pthread, &m->threads[i].ret);
          m->threads[i].state = THREAD_STATE_RELEASED;

          fprintf (stderr, "thread[%d]: released (return value: %p).\n",
                   m->threads[i].index, m->threads[i].ret);
        }
    }

  return 0;
}


