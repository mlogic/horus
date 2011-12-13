/*
 * Copyright (C) 2010  Yasuhiro Ohara
 */

#include <vectorx.h>

/* A vectorx is an array. It is not sorted. The duplications of
   keys are allowed. */

/* The initial size of the array */
#define VECTORX_DEFSIZ 1

/* The number of retries of vectorx_expand () allowed within a
   single vectorx_add () or vectorx_set (). vectorx_add ()
   will give up allocating memory after VECTOR_RETRY times
   of failures. In vectorx_set (), you will be limited to
   use a value of 'index' in the range within 2^{VECTOR_RETRY} 
   far away from the current maximum index. */
#define VECTORX_RETRY 16

int
vectorx_lookup_index (void *data, struct vectorx *v)
{
  int index;

  if (v->size == 0)
    return -1;

  for (index = 0; index < v->size; index++)
    if (v->array[index] == data)
      return index;

  return -1;
}

void *
vectorx_lookup (void *data, struct vectorx *v)
{
  int index;
  index = vectorx_lookup_index (data, v);
  if (index < 0)
    return NULL;
  return v->array[index];
}

static void
vectorx_expand (struct vectorx *v)
{
  void *newarray;

  newarray = (void **)
    realloc (v->array, v->limit * 2 * sizeof (void *));
  if (newarray == NULL)
    return;
  memset ((caddr_t) newarray + v->limit * sizeof (void *), 0,
          v->limit * sizeof (void *));

  v->array = newarray;
  v->limit *= 2;
}

void
vectorx_add (void *data, struct vectorx *v)
{
  int retry = VECTORX_RETRY;

  if (v->size == v->limit)
    {
      while (retry-- && v->size == v->limit)
        vectorx_expand (v);
      if (v->size == v->limit)
        {
          fprintf (stderr, "Cannot double vector size from %d\n", v->limit);
          fprintf (stderr, "Give up to add data %p\n", data);
          return;
        }
    }

  /* add */
  v->array[v->size++] = data;
}

void
vectorx_remove (void *data, struct vectorx *v)
{
  int index;

  index = vectorx_lookup_index (data, v);
  if (index < 0)
    {
      fprintf (stderr, "Can't remove from vector[%p]: no such data: %p\n",
               v, data);
      assert (0);
      return;
    }

  /* remove */
  v->array[index] = NULL;
  v->size--;

  /* shift */
  if (index + 1 < v->limit)
    memmove (&v->array[index], &v->array[index + 1],
             (v->limit - index - 1) * sizeof (void *));
}

void
vectorx_remove_index (int index, struct vectorx *v)
{
  assert (index >= 0);

  /* remove */
  v->array[index] = NULL;
  v->size--;

  /* shift */
  if (index + 1 < v->limit)
    memmove (&v->array[index], &v->array[index + 1],
             (v->limit - index - 1) * sizeof (void *));
}

void
vectorx_clear (struct vectorx *v)
{
  memset (v->array, 0, v->limit * sizeof (void *));
  v->size = 0;
}

/* You will not want to sort the vector when you use below functions */
void
vectorx_set (struct vectorx *v, int index, void *data)
{
  int retry = VECTORX_RETRY;

  if (v->limit <= index)
    {
      while (retry-- && v->limit <= index)
        vectorx_expand (v);
      if (v->limit <= index)
        {
          fprintf (stderr, "Cannot double vector size from %d\n", v->limit);
          fprintf (stderr, "Give up to set data %p at index %d\n", data, index);
          return;
        }
    }

  /* add */
  v->array[index] = data;
  if (v->size <= index)
    v->size = index + 1;
}

void *
vectorx_get (struct vectorx *v, int index)
{
  if (v->size <= index)
    return NULL;
  return v->array[index];
}

int
vectorx_empty_index (struct vectorx *v)
{
  int index;
  for (index = 0; index < v->size; index++)
    if (v->array[index] == NULL)
      return index;
  return index;
}

struct vectorx_node *
vectorx_head (struct vectorx *vector)
{
  struct vectorx_node *node;

  if (vector->size == 0)
    return NULL;

  node = (struct vectorx_node *) malloc (sizeof (struct vectorx_node));
  if (! node)
    return NULL;
  memset (node, 0, sizeof (struct vectorx_node));

  node->vector = vector;
  node->index = 0;
  node->data = node->vector->array[node->index];

  return node;
}

struct vectorx_node *
vectorx_next (struct vectorx_node *node)
{
  /* vector might be deleted. in the case, reload data only.
     if otherwise, proceed the index in the array. */
  if (node->data == node->vector->array[node->index] &&
      node->index < node->vector->size)
    node->index++;

  /* if index reaches end, return NULL */
  if (node->index >= node->vector->size)
    {
      free (node);
      return NULL;
    }

  node->data = node->vector->array[node->index];

  return node;
}

/* for break */
void
vectorx_break (struct vectorx_node *node)
{
  free (node);
}

struct vectorx *
vectorx_create ()
{
  struct vectorx *v;

  v = (struct vectorx *) malloc (sizeof (struct vectorx));
  if (v == NULL)
    return NULL;
  memset (v, 0, sizeof (struct vectorx));
  v->size = 0;
  v->limit = VECTORX_DEFSIZ;
  v->array = (void **) malloc (v->limit * sizeof (void *));
  if (v->array == NULL)
    {
      free (v);
      return NULL;
    }
  memset (v->array, 0, v->limit * sizeof (void *));

  return v;
}

void
vectorx_delete (struct vectorx *v)
{
  free (v->array);
  free (v);
}

void
vectorx_debug (struct vectorx *v)
{
  struct vectorx_node *node;

  fprintf (stderr, "vector[%p]: size = %d limit = %d array = %p\n",
           v, v->size, v->limit, v->array);
  for (node = vectorx_head (v); node; node = vectorx_next (node))
    fprintf (stderr, "index: %d data: %p\n", node->index, node->data);
}

int
vectorx_is_same (struct vectorx *va, struct vectorx *vb)
{
  int i;
  if (va->size != vb->size)
    return 0;

  for (i = 0; i < va->size; i++)
    if (va->array[i] != vb->array[i])
      return 0;

  return 1;
}

int
vectorx_is_empty (struct vectorx *v)
{
  if (v->size == 0)
    return 1;
  return 0;
}

struct vectorx *
vectorx_copy (struct vectorx *v)
{
  struct vectorx *vector;
  int index;
  vector = vectorx_create ();
  for (index = 0; index < v->size; index++)
    vectorx_set (vector, index, vectorx_get (v, index));
  return vector;
}

struct vectorx *
vectorx_common (struct vectorx *va, struct vectorx *vb)
{
  struct vectorx *common = vectorx_create ();
  struct vectorx_node *vna, *vnb;
  for (vna = vectorx_head (va); vna; vna = vectorx_next (vna))
    for (vnb = vectorx_head (vb); vnb; vnb = vectorx_next (vnb))
      if (vna->data && vna->data == vnb->data)
        vectorx_add (vna->data, common);
  return common;
}

struct vectorx *
vectorx_catenate (struct vectorx *dst, struct vectorx *src)
{
  struct vectorx_node *vn;
  for (vn = vectorx_head (src); vn; vn = vectorx_next (vn))
    vectorx_add (vn->data, dst);
  return dst;
}

struct vectorx *
vectorx_merge (struct vectorx *dst, struct vectorx *src)
{
  struct vectorx_node *vn;
  for (vn = vectorx_head (src); vn; vn = vectorx_next (vn))
    if (! vectorx_lookup (vn->data, dst))
      vectorx_add (vn->data, dst);
  return dst;
}


