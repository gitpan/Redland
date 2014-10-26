/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_list.h - RDF List Interface definition
 *
 * $Id: rdf_list.h,v 1.13 2002/06/01 10:39:14 cmdjb Exp $
 *
 * Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 * 
 */



#ifndef LIBRDF_LIST_H
#define LIBRDF_LIST_H

#include <rdf_iterator.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LIBRDF_INTERNAL

/* private structure */
struct librdf_list_node_s
{
  struct librdf_list_node_s* next;
  struct librdf_list_node_s* prev;
  void *data;
};
typedef struct librdf_list_node_s librdf_list_node;


struct librdf_list_s
{
  librdf_world *world;
  librdf_list_node* first;
  librdf_list_node* last;
  int length;
  int (*equals) (void* data1, void *data2);
};

#endif

librdf_list* librdf_new_list(librdf_world *world);
void librdf_free_list(librdf_list* list);

void librdf_list_clear(librdf_list* list);
/* add to end of list (push) */
int librdf_list_add(librdf_list* list, void *data);
/* add to start of list */
int librdf_list_unshift(librdf_list* list, void *data);
/* remove from start of list */
void* librdf_list_shift(librdf_list* list);
/* remove from end of list (pop) */
void* librdf_list_pop(librdf_list* list);
void *librdf_list_remove(librdf_list* list, void *data);
int librdf_list_contains(librdf_list* list, void *data);
int librdf_list_size(librdf_list* list);

void librdf_list_set_equals(librdf_list* list, int (*equals) (void* data1, void *data2));

librdf_iterator* librdf_list_get_iterator(librdf_list* list);

void librdf_list_foreach(librdf_list* list, void (*fn)(void *, void *), void *user_data);

#ifdef __cplusplus
}
#endif

#endif
