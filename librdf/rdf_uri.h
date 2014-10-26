/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_uri.h - RDF URI Definition
 *
 * $Id: rdf_uri.h,v 1.22 2003/06/04 20:17:49 cmdjb Exp $
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



#ifndef LIBRDF_URI_H
#define LIBRDF_URI_H

#include <rdf_digest.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef LIBRDF_INTERNAL

struct librdf_uri_s
{
  librdf_world *world;
  char *string;
  int string_length; /* useful for fast comparisons (that fail) */
  int usage;
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  int max_usage;
#endif
};

#endif

/* class methods */
void librdf_init_uri(librdf_world *world);
void librdf_finish_uri(librdf_world *world);

/* constructors */
librdf_uri* librdf_new_uri (librdf_world *world, const char *string);
/* Create a new URI from an existing URI - CLONE */
librdf_uri* librdf_new_uri_from_uri (librdf_uri* uri);
/* Create a new URI from an existing URI and local name */
librdf_uri* librdf_new_uri_from_uri_local_name (librdf_uri* uri, const char *local_name);

/* destructor */
void librdf_free_uri(librdf_uri *uri);

/* methods */
char* librdf_uri_as_string (librdf_uri *uri);
char* librdf_uri_as_counted_string (librdf_uri *uri, size_t *len_p);
librdf_digest* librdf_uri_get_digest (librdf_uri *uri);
void librdf_uri_print (librdf_uri* uri, FILE *fh);
char* librdf_uri_to_string (librdf_uri* uri);
char* librdf_uri_to_counted_string (librdf_uri* uri, size_t* len_p);
int librdf_uri_equals(librdf_uri* first_uri, librdf_uri* second_uri);
int librdf_uri_is_file_uri(librdf_uri* uri);
const char* librdf_uri_to_filename(librdf_uri* uri);
librdf_uri* librdf_new_uri_normalised_to_base(const char *uri_string, librdf_uri* source_uri, librdf_uri* base_uri);
librdf_uri* librdf_new_uri_relative_to_base(librdf_uri* base_uri, const char *uri_string);
librdf_uri* librdf_new_uri_from_filename(librdf_world* world, const char *filename);

#ifdef __cplusplus
}
#endif

#endif
