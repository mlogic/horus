/*
 * Copyright (C) 2010  Yasuhiro Ohara
 */

#ifndef _VECTORX_H_
#define _VECTORX_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

struct vectorx_node
{
  struct vectorx *vector;
  int index;
  void *data;
};
typedef struct vectorx_node *vectorx_node_t;

struct vectorx
{
  void **array;
  unsigned int limit;
  unsigned int size;
};
typedef struct vectorx *vectorx_t;

int vectorx_lookup_index (void *data, struct vectorx *v);
void *vectorx_lookup (void *data, struct vectorx *v);

void vectorx_add (void *data, struct vectorx *v);
void vectorx_remove (void *data, struct vectorx *v);
void vectorx_remove_index (int index, struct vectorx *v);
void vectorx_clear (struct vectorx *v);

void vectorx_set (struct vectorx *v, int index, void *data);
void *vectorx_get (struct vectorx *v, int index);
int vectorx_empty_index (struct vectorx *v);

struct vectorx_node *vectorx_head (struct vectorx *vector);
struct vectorx_node *vectorx_next (struct vectorx_node *node);
void vectorx_break (struct vectorx_node *node);

struct vectorx *vectorx_create ();
void vectorx_delete (struct vectorx *v);

void vectorx_debug (struct vectorx *v);

int vectorx_is_same (struct vectorx *va, struct vectorx *vb);
int vectorx_is_empty (struct vectorx *v);

struct vectorx *vectorx_copy (struct vectorx *v);
struct vectorx *vectorx_common (struct vectorx *va, struct vectorx *vb);
struct vectorx *vectorx_catenate (struct vectorx *dst, struct vectorx *src);
struct vectorx *vectorx_merge (struct vectorx *dst, struct vectorx *src);

#endif /*_VECTORX_H_*/

