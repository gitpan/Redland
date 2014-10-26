/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_uri.c - RDF URI Implementation
 *
 * $Id: rdf_uri.c,v 1.55 2003/08/30 21:51:14 cmdjb Exp $
 *
 * Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
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
#include <string.h>
#include <ctype.h> /* for isalnum */
#ifdef WITH_THREADS
#include <pthread.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

#include <librdf.h>


/* for raptor_uri_resolve_uri_reference */
#define RAPTOR_IN_REDLAND
#include <raptor.h>


#ifndef STANDALONE

/* class methods */


/**
 * librdf_init_uri - Initialise the librdf_uri class
 * @world: redland world object
 *
 **/
void
librdf_init_uri(librdf_world *world)
{
  /* If no default given, create an in memory hash */
  if(!world->uris_hash) {
    world->uris_hash=librdf_new_hash(world, NULL);
    if(!world->uris_hash)
      LIBRDF_FATAL1(world, librdf_init_uri,
                    "Failed to create URI hash from factory");
    
    if(librdf_hash_open(world->uris_hash, NULL, 0, 1, 1, NULL))
      LIBRDF_FATAL1(world, librdf_init_uri, "Failed to open URI hash");

    /* remember to free it later */
    world->uris_hash_allocated_here=1;
  }
}



/**
 * librdf_finish_uri - Terminate the librdf_uri class
 * @world: redland world object
 **/
void
librdf_finish_uri(librdf_world *world)
{
  librdf_hash_close(world->uris_hash);

  if(world->uris_hash_allocated_here)
    librdf_free_hash(world->uris_hash);
}



/**
 * librdf_new_uri - Constructor - create a new librdf_uri object from a URI string
 * @world: redland world object
 * @uri_string: URI in string form
 * 
 * A new URI is constructed from a copy of the string.
 *
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri (librdf_world *world, 
                const char *uri_string)
{
  librdf_uri* new_uri;
  char *new_string;
  int length;
  librdf_hash_datum key, value; /* on stack - not allocated */
  librdf_hash_datum *old_value;

  if(!uri_string)
    return NULL;

#ifdef WITH_THREADS
  pthread_mutex_lock(world->mutex);
#endif
  
  length=strlen(uri_string);

  key.data=(char*)uri_string;
  key.size=length;

  /* if existing URI found in hash, return it */
  if((old_value=librdf_hash_get_one(world->uris_hash, &key))) {
    new_uri=*(librdf_uri**)old_value->data;

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
    LIBRDF_DEBUG3(librdf_new_uri, "Found existing URI %s in hash with current usage %d\n", uri_string, new_uri->usage);
#endif

    librdf_free_hash_datum(old_value);
    new_uri->usage++;

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
    if(new_uri->usage > new_uri->max_usage)
      new_uri->max_usage=new_uri->usage;
#endif    

    goto unlock;
  }
  

  /* otherwise create a new one */

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG2(librdf_new_uri, "Creating new URI %s in hash\n", uri_string);
#endif

  new_uri = (librdf_uri*)LIBRDF_CALLOC(librdf_uri, 1, sizeof(librdf_uri));
  if(!new_uri)
    goto unlock;

  new_uri->world=world;
  new_uri->string_length=length;

  new_string=(char*)LIBRDF_MALLOC(librdf_uri, length+1);
  if(!new_string) {
    LIBRDF_FREE(librdf_uri, new_uri);
    new_uri=NULL;
    goto unlock;
  }
  
  strcpy(new_string, uri_string);
  new_uri->string=new_string;

  new_uri->usage=1;
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  new_uri->max_usage=1;
#endif

  value.data=&new_uri; value.size=sizeof(librdf_uri*);
  
  /* store in hash: URI-string => (librdf_uri*) */
  if(librdf_hash_put(world->uris_hash, &key, &value)) {
    LIBRDF_FREE(librdf_uri, new_string);
    LIBRDF_FREE(librdf_uri, new_uri);
    new_uri=NULL;
  }

 unlock:
#ifdef WITH_THREADS
  pthread_mutex_unlock(world->mutex);
#endif

  return new_uri;
}


/**
 * librdf_new_uri_from_uri - Copy constructor - create a new librdf_uri object from an existing librdf_uri object
 * @old_uri: &librdf_uri object
 * 
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri_from_uri (librdf_uri* old_uri) {
  old_uri->usage++;
  return old_uri;
}


/**
 * librdf_new_uri_from_uri_local_name - Copy constructor - create a new librdf_uri object from an existing librdf_uri object and a local name
 * @old_uri: &librdf_uri object
 * @local_name: local name to append to URI
 * 
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri_from_uri_local_name (librdf_uri* old_uri, const char *local_name) {
  int len=old_uri->string_length + strlen(local_name) +1 ; /* +1 for \0 */
  char *new_string;
  librdf_uri* new_uri;

  if(!old_uri)
    return NULL;
  
  new_string=(char*)LIBRDF_CALLOC(cstring, 1, len);
  if(!new_string)
    return NULL;

  strcpy(new_string, old_uri->string);
  strcat(new_string, local_name);

  new_uri=librdf_new_uri (old_uri->world, new_string);
  LIBRDF_FREE(cstring, new_string);

  return new_uri;
}


/**
 * librdf_new_uri_normalised_to_base - Constructor - create a new librdf_uri object from a URI string stripped of the source URI, made relative to the base URI
 * @uri_string: URI in string form
 * @source_uri: source URI to remove
 * @base_uri: base URI to add
 * 
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri_normalised_to_base(const char *uri_string,
                                  librdf_uri* source_uri,
                                  librdf_uri* base_uri) 
{
  int uri_string_len;
  int len;
  char *new_uri_string;
  librdf_uri *new_uri;
  librdf_world *world=source_uri->world;
                                    
  if(!uri_string)
    return NULL;

  /* empty URI - easy, just make from base_uri */
  if(!*uri_string && base_uri)
    return librdf_new_uri_from_uri(base_uri);

  /* not a fragment, and no match - easy */
  if(*uri_string != '#' &&
     strncmp(uri_string, source_uri->string, source_uri->string_length))
    return librdf_new_uri(world, uri_string);

  /* darn - is a fragment or matches, is a prefix of the source URI */

  /* move uri_string pointer to first non-matching char 
   * unless a fragment, when all of the uri_string will 
   * be appended
   */
  if(*uri_string != '#')
    uri_string += source_uri->string_length;

  /* size of remaining bytes to copy from uri_string */
  uri_string_len=strlen(uri_string);

  /* total bytes */
  len=uri_string_len + 1 + base_uri->string_length;

  new_uri_string=(char*)LIBRDF_MALLOC(cstring, len);
  if(!new_uri_string)
    return NULL;
  strncpy(new_uri_string, base_uri->string, base_uri->string_length);
  /* strcpy not strncpy since I want a \0 on the end */
  strcpy(new_uri_string + base_uri->string_length, uri_string);
  
  new_uri=librdf_new_uri(world, new_uri_string);
  LIBRDF_FREE(cstring, new_uri_string); /* always free this even on failure */

  return new_uri; /* new URI or NULL from librdf_new_uri failure */
}



/**
 * librdf_new_uri_relative_to_base - Constructor - create a new librdf_uri object from a URI string relative to a base URI
 * @base_uri: absolute base URI
 * @uri_string: relative URI string
 *
 * An empty uri_string or NULL is equivalent to 
 * librdf_new_uri_from_uri(base_uri)
 * 
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri_relative_to_base(librdf_uri* base_uri,
                                const char *uri_string) {
  char *buffer;
  int buffer_length;
  librdf_uri* new_uri;
  librdf_world *world=base_uri->world;
                                  
  if(!uri_string)
    return NULL;
  
  /* If URI string is empty, just copy base URI */
  if(!*uri_string)
    return librdf_new_uri_from_uri(base_uri);
  
  buffer_length=base_uri->string_length + strlen(uri_string) +1;
  buffer=(char*)LIBRDF_MALLOC(cstring, buffer_length);
  if(!buffer)
    return NULL;
  
  raptor_uri_resolve_uri_reference(base_uri->string, uri_string,
                                   buffer, buffer_length);

  new_uri=librdf_new_uri(world, buffer);
  LIBRDF_FREE(cstring, buffer);
  return new_uri;
}


/**
 * librdf_new_uri_from_filename - Constructor - create a new librdf_uri object from a filename
 * @world: Redland &librdf_world object
 * @filename: filename
 *
 * Return value: a new &librdf_uri object or NULL on failure
 **/
librdf_uri*
librdf_new_uri_from_filename(librdf_world* world, const char *filename) {
  librdf_uri* new_uri;
  char *uri_string;

  if(!filename)
    return NULL;
  
  uri_string=raptor_uri_filename_to_uri_string(filename);
  if(!uri_string)
    return NULL;
  
  new_uri=librdf_new_uri(world, uri_string);
  LIBRDF_FREE(cstring, uri_string);
  return new_uri;
}



/**
 * librdf_free_uri - Destructor - destroy a librdf_uri object
 * @uri: &librdf_uri object
 * 
 **/
void
librdf_free_uri (librdf_uri* uri) 
{
  librdf_hash_datum key; /* on stack */
#ifdef WITH_THREADS
  librdf_world *world = uri->world;
#endif

#ifdef WITH_THREADS
  pthread_mutex_lock(world->mutex);
#endif

  uri->usage--;
  
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG3(librdf_free_uri, "URI %s usage count now %d\n", uri->string, uri->usage);
#endif

  /* decrement usage, don't free if not 0 yet*/
  if(uri->usage) {
#ifdef WITH_THREADS
    pthread_mutex_unlock(world->mutex);
#endif
    return;
  }

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG3(librdf_free_uri, "Deleting URI %s from hash, max usage was %d\n", uri->string, uri->max_usage);
#endif

  key.data=uri->string;
  key.size=uri->string_length;
  if(librdf_hash_delete_all(uri->world->uris_hash, &key) )
    LIBRDF_FATAL1(world, librdf_free_uri, "Hash deletion failed");


  if(uri->string)
    LIBRDF_FREE(cstring, uri->string);
  LIBRDF_FREE(librdf_uri, uri);

#ifdef WITH_THREADS
  pthread_mutex_unlock(world->mutex);
#endif
}


/**
 * librdf_uri_as_string - Get a pointer to the string representation of the URI
 * @uri: &librdf_uri object
 * 
 * Returns a shared pointer to the URI string representation. 
 * Note: does not allocate a new string so the caller must not free it.
 * 
 * Return value: string representation of URI
 **/
char*
librdf_uri_as_string (librdf_uri *uri) 
{
  return uri->string;
}


/**
 * librdf_uri_as_counted_string - Get a pointer to the string representation of the URI with length
 * @uri: &librdf_uri object
 * @len_p: pointer to location to store length
 * 
 * Returns a shared pointer to the URI string representation. 
 * Note: does not allocate a new string so the caller must not free it.
 * 
 * Return value: string representation of URI
 **/
char*
librdf_uri_as_counted_string(librdf_uri *uri, size_t* len_p) 
{
  if(len_p)
    *len_p=uri->string_length;
  return uri->string;
}


/**
 * librdf_uri_get_digest - Get a digest for the URI
 * @uri: &librdf_uri object
 * 
 * Generates a digest object for the URI.  The digest factory used is
 * determined at class initialisation time by librdf_init_uri().
 * 
 * Return value: new &librdf_digest object or NULL on failure.
 **/
librdf_digest*
librdf_uri_get_digest (librdf_uri* uri) 
{
  librdf_world *world=uri->world;
  librdf_digest* d;
  
  d=librdf_new_digest_from_factory(world, world->digest_factory);
  if(!d)
    return NULL;
  
  librdf_digest_init(d);
  librdf_digest_update(d, (unsigned char*)uri->string, uri->string_length);
  librdf_digest_final(d);
  
  return d;
}


/**
 * librdf_uri_print - Print the URI to the given file handle
 * @uri: &librdf_uri object
 * @fh: &FILE handle
 **/
void
librdf_uri_print (librdf_uri* uri, FILE *fh) 
{
  fputs(uri->string, fh);
}


/**
 * librdf_uri_to_string - Format the URI as a string
 * @uri: &librdf_uri object
 * 
 * Note: this method allocates a new string since this is a _to_ method
 * and the caller must free the resulting memory.
 *
 * Return value: string representation of the URI or NULL on failure
 **/
char*
librdf_uri_to_string (librdf_uri* uri)
{
  return librdf_uri_to_counted_string(uri, NULL);
}


/**
 * librdf_uri_to_counted_string - Format the URI as a counted string
 * @uri: &librdf_uri object
 * @len_p: pointer to location to store length
 * 
 * Note: this method allocates a new string since this is a _to_ method
 * and the caller must free the resulting memory.
 *
 * Return value: string representation of the URI or NULL on failure
 **/
char*
librdf_uri_to_counted_string (librdf_uri* uri, size_t* len_p)
{
  char *s;

  if(!uri)
    return NULL;
  
  if(len_p)
    *len_p=uri->string_length;

  s=(char*)LIBRDF_MALLOC(cstring, uri->string_length+1);
  if(!s)
    return NULL;

  strcpy(s, uri->string);
  return s;
}


/**
 * librdf_uri_equals - Compare two librdf_uri objects for equality
 * @first_uri: &librdf_uri object 1
 * @second_uri: &librdf_uri object 2
 * 
 * Return value: non 0 if the objects are equal
 **/
int
librdf_uri_equals(librdf_uri* first_uri, librdf_uri* second_uri) 
{
  if(!first_uri || !second_uri)
    return 0;
  return (first_uri == second_uri);
}


/**
 * librdf_uri_is_file_uri - Test if a URI points to a filename
 * @uri: &librdf_uri object
 * 
 * Return value: 0 if the URI points to a file
 **/
int
librdf_uri_is_file_uri(librdf_uri* uri) 
{
  return raptor_uri_is_file_uri(uri->string);
}


/**
 * librdf_uri_to_filename - Return pointer to filename of URI
 * @uri: &librdf_uri object
 * 
 * Returns a pointer to a newly allocated buffer that
 * the caller must free.  This will fail if the URI
 * is not a file: URI.  This can be checked with &librdf_uri_is_file_uri
 *
 * Return value: pointer to filename or NULL on failure
 **/
const char*
librdf_uri_to_filename(librdf_uri* uri) 
{
  return raptor_uri_uri_string_to_filename(uri->string);
  
}

#endif


/* TEST CODE */


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  char *hp_string="http://www.ilrt.bristol.ac.uk/people/cmdjb/";
  librdf_uri *uri1, *uri2, *uri3, *uri4, *uri5, *uri6, *uri7, *uri8, *uri9;
  librdf_digest *d;
  char *program=argv[0];
  const char *file_string="/big/long/directory/file";
  const char *file_uri_string=  "file:///big/long/directory/file";
  const char *uri_string=  "http://example.com/big/long/directory/blah#frag";
  const char *relative_uri_string1="#foo";
  const char *relative_uri_string2="bar";
  librdf_world *world;
  
  world=librdf_new_world();
  librdf_world_init_mutex(world);

  librdf_init_digest(world);
  librdf_init_hash(world);
  librdf_init_uri(world);

  fprintf(stderr, "%s: Creating new URI from string\n", program);
  uri1=librdf_new_uri(world, hp_string);
  if(!uri1) {
    fprintf(stderr, "%s: Failed to create URI from string '%s'\n", program, 
	    hp_string);
    return(1);
  }
  
  fprintf(stderr, "%s: Home page URI is ", program);
  librdf_uri_print(uri1, stderr);
  fputs("\n", stderr);
  
  fprintf(stderr, "%s: Creating URI from URI\n", program);
  uri2=librdf_new_uri_from_uri(uri1);
  if(!uri2) {
    fprintf(stderr, "%s: Failed to create new URI from old one\n", program);
    return(1);
  }

  fprintf(stderr, "%s: New URI is ", program);
  librdf_uri_print(uri2, stderr);
  fputs("\n", stderr);

  
  fprintf(stderr, "%s: Getting digest for URI\n", program);
  d=librdf_uri_get_digest(uri2);
  if(!d) {
    fprintf(stderr, "%s: Failed to get digest for URI %s\n", program, 
	    librdf_uri_as_string(uri2));
    return(1);
  }
  fprintf(stderr, "%s: Digest is: ", program);
  librdf_digest_print(d, stderr);
  fputs("\n", stderr);
  librdf_free_digest(d);

  uri3=librdf_new_uri(world, "file:/big/long/directory/");
  uri4=librdf_new_uri(world, "http://somewhere/dir/");
  fprintf(stderr, "%s: Source URI is ", program);
  librdf_uri_print(uri3, stderr);
  fputs("\n", stderr);
  fprintf(stderr, "%s: Base URI is ", program);
  librdf_uri_print(uri4, stderr);
  fputs("\n", stderr);
  fprintf(stderr, "%s: URI string is '%s'\n", program, uri_string);

  uri5=librdf_new_uri_normalised_to_base(uri_string, uri3, uri4);
  fprintf(stderr, "%s: Normalised URI is ", program);
  librdf_uri_print(uri5, stderr);
  fputs("\n", stderr);


  uri6=librdf_new_uri_relative_to_base(uri5, relative_uri_string1);
  fprintf(stderr, "%s: URI + Relative URI %s gives ", program, 
          relative_uri_string1);
  librdf_uri_print(uri6, stderr);
  fputs("\n", stderr);

  uri7=librdf_new_uri_relative_to_base(uri5, relative_uri_string2);
  fprintf(stderr, "%s: URI + Relative URI %s gives ", program, 
          relative_uri_string2);
  librdf_uri_print(uri7, stderr);
  fputs("\n", stderr);

  uri8=librdf_new_uri_from_filename(world, file_string);
  uri9=librdf_new_uri(world, file_uri_string);
  if(!librdf_uri_equals(uri8, uri9)) {
    fprintf(stderr, "%s: URI string from filename %s returned %s, expected %s\n", program, file_string, librdf_uri_as_string(uri8), file_uri_string);
    return(1);
  }

  fprintf(stderr, "%s: Freeing URIs\n", program);
  librdf_free_uri(uri1);
  librdf_free_uri(uri2);
  librdf_free_uri(uri3);
  librdf_free_uri(uri4);
  librdf_free_uri(uri5);
  librdf_free_uri(uri6);
  librdf_free_uri(uri7);
  librdf_free_uri(uri8);
  librdf_free_uri(uri9);
  
  librdf_finish_uri(world);
  librdf_finish_hash(world);
  librdf_finish_digest(world);

  LIBRDF_FREE(librdf_world, world);

  /* keep gcc -Wall happy */
  return(0);
}

#endif
