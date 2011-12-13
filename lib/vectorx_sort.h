/*
 * Copyright (C) 2010  Yasuhiro Ohara
 */

#ifndef _VECTORX_SORT_H_
#define _VECTORX_SORT_H_

#include <vectorx.h>

typedef int (*vectorx_cmp_t) (const void *, const void *);

int vectorx_compare (const void *a, const void *b);

void vectorx_sort (vectorx_cmp_t cmp, struct vectorx *v);

int vectorx_lookup_index_bsearch (void *data, vectorx_cmp_t cmp,
                                 struct vectorx *v);
void *vectorx_lookup_bsearch (void *data, vectorx_cmp_t cmp,
                             struct vectorx *v);

#endif /*_VECTORX_SORT_H_*/

