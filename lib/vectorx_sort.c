/*
 * Copyright (C) 2010  Yasuhiro Ohara
 */

#include <vectorx.h>
#include <vectorx_sort.h>

/* sample compare function which compares memory address */
int
vectorx_compare (const void *a, const void *b)
{
  void *va = *(void **) a;
  void *vb = *(void **) b;
  if (va == vb)
    return 0;
  if (va == NULL)
    return 1;
  if (vb == NULL)
    return -1;
  return (va < vb ? -1 : 1);
}

void
vectorx_sort (vectorx_cmp_t cmp, struct vectorx *v)
{
  int retval;

  assert (cmp);
  retval = 0;

  /* sort */
#ifdef HAVE_HEAPSORT
  retval = heapsort (v->array, v->size, sizeof (void *), cmp);
  if (retval != 0)
    fprintf (stderr, "heapsort failed: %s\n", strerror (errno));
#else /*HAVE_HEAPSORT*/
  qsort (v->array, v->size, sizeof (void *), cmp);
#endif /*HAVE_HEAPSORT*/
}

/* returns index for data using binary search (recursive function) */
static int
vectorx_binsearch (void *data, int index, int size,
                   vectorx_cmp_t cmp, struct vectorx *v)
{
  int middle = index + size / 2;
  int shift = (size % 2 == 1 ? 1 : 0);
  int ret;

  if (size < 0)
    assert (0);

  ret = (*cmp) (&data, &v->array[middle]);
  if (ret == 0)
    return middle;
  else if (size == 0 || size == 1)
    return -1; /* search failed */
  else if (ret < 0)
    return vectorx_binsearch (data, index, size / 2, cmp, v);
  else
    return vectorx_binsearch (data, middle + shift, size / 2, cmp, v);

  /* not reached */
  return -1;
}

int
vectorx_lookup_index_bsearch (void *data, vectorx_cmp_t cmp,
                              struct vectorx *v)
{
  int index;
  if (v->size == 0)
    return -1;
  index = vectorx_binsearch (data, 0, v->size, cmp, v);
  if (index < 0)
    return -1;
  if (cmp (&data, &v->array[index]) == 0)
    return index;
  return -1;
}

void *
vectorx_lookup_bsearch (void *data, vectorx_cmp_t cmp,
                       struct vectorx *v)
{
  int index;
  index = vectorx_lookup_index_bsearch (data, cmp, v);
  if (index < 0)
    return NULL;
  return v->array[index];
}


