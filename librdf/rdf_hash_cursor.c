/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_hash.c - RDF Hash Cursor Implementation
 *
 * $Id: rdf_hash_cursor.c,v 1.8 2002/08/19 16:32:00 cmdjb Exp $
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


#include <rdf_config.h>

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>

#include <librdf.h>


/* private structure */
struct librdf_hash_cursor_s {
  librdf_hash *hash;
  void *context;
};



/**
 * librdf_new_hash_cursor - Constructor - Create a new hash cursor over a hash
 * @hash: the hash object
 *
 * Return value: a new &librdf_hash_cursor or NULL on failure
 **/
librdf_hash_cursor*
librdf_new_hash_cursor (librdf_hash* hash) 
{
  librdf_hash_cursor* cursor;
  void *cursor_context;

  cursor=(librdf_hash_cursor*)LIBRDF_CALLOC(librdf_hash_cursor, 1, 
                                            sizeof(librdf_hash_cursor));
  if(!cursor)
    return NULL;

  cursor_context=(char*)LIBRDF_CALLOC(librdf_hash_cursor_context, 1,
                                      hash->factory->cursor_context_length);
  if(!cursor_context) {
    LIBRDF_FREE(librdf_hash_cursor, cursor);
    return NULL;
  }

  cursor->hash=hash;
  cursor->context=cursor_context;

  if(hash->factory->cursor_init(cursor->context, hash->context)) {
    librdf_free_hash_cursor(cursor);
    cursor=NULL;
  }

  return cursor;
}


/**
 * librdf_free_hash_cursor - Destructor - destroy a librdf_hash_cursor object
 *
 * @cursor: hash cursor object
 **/
void
librdf_free_hash_cursor (librdf_hash_cursor* cursor) 
{
  if(cursor->context) {
    cursor->hash->factory->cursor_finish(cursor->context);
    LIBRDF_FREE(librdf_hash_cursor_context, cursor->context);
  }

  LIBRDF_FREE(librdf_hash_cursor, cursor);
}


int
librdf_hash_cursor_set(librdf_hash_cursor *cursor,
                       librdf_hash_datum *key,
                       librdf_hash_datum *value)
{
  return cursor->hash->factory->cursor_get(cursor->context, key, value, 
                                           LIBRDF_HASH_CURSOR_SET);
}


int
librdf_hash_cursor_get_next_value(librdf_hash_cursor *cursor, 
                                  librdf_hash_datum *key,
                                  librdf_hash_datum *value)
{
  return cursor->hash->factory->cursor_get(cursor->context, key, value, 
                                           LIBRDF_HASH_CURSOR_NEXT_VALUE);
}


int
librdf_hash_cursor_get_first(librdf_hash_cursor *cursor,
                             librdf_hash_datum *key, librdf_hash_datum *value)
{
  return cursor->hash->factory->cursor_get(cursor->context, key, value, 
                                           LIBRDF_HASH_CURSOR_FIRST);
}


int
librdf_hash_cursor_get_next(librdf_hash_cursor *cursor, librdf_hash_datum *key,
                            librdf_hash_datum *value)
{
  return cursor->hash->factory->cursor_get(cursor->context, key, value, 
                                           LIBRDF_HASH_CURSOR_NEXT);
}
