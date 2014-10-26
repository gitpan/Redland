/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_digest.c - RDF Digest Factory / Digest implementation
 *
 * $Id: rdf_digest.c,v 1.38 2003/08/27 10:36:26 cmdjb Exp $
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
#include <stdarg.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

#include <librdf.h>

#ifndef STANDALONE
/* prototypes for helper functions */
static void librdf_delete_digest_factories(librdf_world* world);


/* helper functions */
static void
librdf_delete_digest_factories(librdf_world *world)
{
  librdf_digest_factory *factory, *next;
  
  for(factory=world->digests; factory; factory=next) {
    next=factory->next;
    LIBRDF_FREE(librdf_digest_factory, factory->name);
    LIBRDF_FREE(librdf_digest_factory, factory);
  }
  world->digests=NULL;
}


/**
 * librdf_digest_register_factory - Register a hash factory
 * @world: redland world object
 * @name: the name of the hash
 * @factory: function to be called to register the factory parameters
 * 
 **/
void
librdf_digest_register_factory(librdf_world *world, const char *name,
			       void (*factory) (librdf_digest_factory*))
{
  librdf_digest_factory *d, *digest;
  char *name_copy;

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG2(librdf_digest_register_factory,
		"Received registration for digest %s\n", name);
#endif
  digest=(librdf_digest_factory*)LIBRDF_CALLOC(librdf_digest_factory, 1,
					       sizeof(librdf_digest_factory));
  if(!digest)
    LIBRDF_FATAL1(world, librdf_digest_register_factory, "Out of memory");
  
  name_copy=(char*)LIBRDF_CALLOC(cstring, 1, strlen(name)+1);
  if(!name_copy) {
    LIBRDF_FREE(librdf_digest, digest);
    LIBRDF_FATAL1(world, librdf_digest_register_factory, "Out of memory");
  }
  strcpy(name_copy, name);
  digest->name=name_copy;
        
  for(d = world->digests; d; d = d->next ) {
    if(!strcmp(d->name, name_copy)) {
      LIBRDF_ERROR2(world, librdf_digest_register_factory,
		    "digest %s already registered", d->name);
      return;
    }
  }

  
        
  /* Call the digest registration function on the new object */
  (*factory)(digest);


#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG4(librdf_digest_register_factory,
		"%s has context size %d and digest size %d\n", name,
		digest->context_length, digest->digest_length);
#endif
  
  digest->next = world->digests;
  world->digests = digest;
  
}


/**
 * librdf_get_digest_factory - get a digest factory
 * @world: redland world object
 * @name: the name of the factory
 * 
 * Return value: the factory or NULL if not found
 **/
librdf_digest_factory*
librdf_get_digest_factory(librdf_world *world, const char *name) 
{
  librdf_digest_factory *factory;

  /* return 1st digest if no particular one wanted - why? */
  if(!name) {
    factory=world->digests;
    if(!factory) {
      LIBRDF_DEBUG1(librdf_get_digest_factory, "No digests available\n");
      return NULL;
    }
  } else {
    for(factory=world->digests; factory; factory=factory->next) {
      if(!strcmp(factory->name, name)) {
	break;
      }
    }
    /* else FACTORY with name digest_name not found */
    if(!factory)
      return NULL;
  }
  
  return factory;
}


/**
 * librdf_new_digest - Constructor - create a new librdf_digest object
 * @world: redland world object
 * @name: the digest name to use to create this digest
 * 
 * Return value: new &librdf_digest object or NULL
 **/
librdf_digest*
librdf_new_digest(librdf_world *world, char *name)
{
  librdf_digest_factory* factory;
  
  factory=librdf_get_digest_factory(world, name);
  if(!factory)
    return NULL;
  
  return librdf_new_digest_from_factory(world, factory);
}


/**
 * librdf_new_digest_from_factory - Constructor - create a new librdf_digest object
 * @world: redland world object
 * @factory: the digest factory to use to create this digest
 * 
 * Return value: new &librdf_digest object or NULL
 **/
librdf_digest*
librdf_new_digest_from_factory(librdf_world *world,
                               librdf_digest_factory *factory)
{
  librdf_digest* d;

  d=(librdf_digest*)LIBRDF_CALLOC(librdf_digest, 1, sizeof(librdf_digest));
  if(!d)
    return NULL;
        
  d->context=(char*)LIBRDF_CALLOC(digest_context, 1, factory->context_length);
  if(!d->context) {
    librdf_free_digest(d);
    return NULL;
  }
        
  d->digest=(unsigned char*)LIBRDF_CALLOC(digest_digest, 1,
					  factory->digest_length);
  if(!d->digest) {
    librdf_free_digest(d);
    return NULL;
  }
  
  d->factory=factory;
  
  return d;
}


/**
 * librdf_free_digest - Destructor- destroy a librdf_digest object
 * @digest: the digest
 * 
 **/
void
librdf_free_digest(librdf_digest *digest) 
{
  if(digest->context)
    LIBRDF_FREE(digest_context, digest->context);
  if(digest->digest)
    LIBRDF_FREE(digest_digest, digest->digest);
  LIBRDF_FREE(librdf_digest, digest);
}



/* methods */

/**
 * librdf_digest_init - (Re)initialise the librdf_digest object
 * @digest: the digest
 * 
 **/
void
librdf_digest_init(librdf_digest* digest) 
{
  digest->factory->init(digest->context);
}


/**
 * librdf_digest_update - Add more data to the librdf_digest object
 * @digest: the digest
 * @buf: the data buffer
 * @length: the length of the data
 * 
 **/
void
librdf_digest_update(librdf_digest* digest, unsigned char *buf, size_t length) 
{
  digest->factory->update(digest->context, buf, length);
}


/**
 * librdf_digest_final - Finish the digesting of data
 * @digest: the digest
 * 
 * The digest can now be returned via librdf_digest_get_digest().
 **/
void
librdf_digest_final(librdf_digest* digest) 
{
  void* digest_data;
  
  digest->factory->final(digest->context);
  
  digest_data=(*(digest->factory->get_digest))(digest->context);
  memcpy(digest->digest, digest_data, digest->factory->digest_length);
}


/**
 * librdf_digest_get_digest - Get the calculated digested value.
 * @digest: the digest
 * 
 * Return value: pointer to the memory containing the digest.  It will
 * be digest_factory->digest_length bytes in length.
 *
 **/
void*
librdf_digest_get_digest(librdf_digest* digest)
{
  return digest->digest;
}


/**
 * librdf_digest_to_string - Get a string representation of the digest object
 * @digest: the digest
 * 
 * Return value: a newly allocated string that represents the digest.
 * This must be released by the caller using free() 
 **/
char *
librdf_digest_to_string(librdf_digest* digest)
{
  unsigned char* data=digest->digest;
  int mdlen=digest->factory->digest_length;
  char* b;
  int i;
        
  b=(char*)LIBRDF_MALLOC(cstring, 1+(mdlen<<1));
  if(!b)
    LIBRDF_FATAL1(digest->world, librdf_digest_to_string, "Out of memory");
  
  for(i=0; i<mdlen; i++)
    sprintf(b+(i<<1), "%02x", (unsigned int)data[i]);
  b[i<<1]='\0';
  
  return(b);
}


/**
 * librdf_digest_print - Print the digest to a FILE handle
 * @digest: the digest
 * @fh: file handle
 *
 **/
void
librdf_digest_print(librdf_digest* digest, FILE* fh)
{
  char *s=librdf_digest_to_string(digest);
  
  if(!s)
    return;
  fputs(s, fh);
  LIBRDF_FREE(cstring, s);
}


/**
 * librdf_init_digest - Initialise the librdf_digest class
 * @world: redland world object
 **/
void
librdf_init_digest(librdf_world *world) 
{
#ifdef HAVE_OPENSSL_DIGESTS
  librdf_digest_openssl_constructor(world);
#endif
#ifdef HAVE_LOCAL_MD5_DIGEST
  librdf_digest_md5_constructor(world);
#endif
#ifdef HAVE_LOCAL_RIPEMD160_DIGEST
  librdf_digest_rmd160_constructor(world);
#endif
#ifdef HAVE_LOCAL_SHA1_DIGEST
  librdf_digest_sha1_constructor(world);
#endif

  /* set default */
  world->digest_factory=librdf_get_digest_factory(world,
                                                  world->digest_factory_name);
  
}


/**
 * librdf_finish_digest - Terminate the librdf_digest class
 * @world: redland world object
 **/
void
librdf_finish_digest(librdf_world *world) 
{
  librdf_delete_digest_factories(world);
}
#endif


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);

struct t
{
  char *type;
  char *result;
};

int
main(int argc, char *argv[]) 
{
  librdf_digest* d;
  char *test_data="http://www.ilrt.bristol.ac.uk/people/cmdjb/";
  struct t test_data_answers[]={
    {"MD5", "78f68989ea3ded775925a1a00fe49abe"},
    {"SHA1", "0452bca820dfdaa707f0278216c598f185867993"},
    {"RIPEMD160", "75c5de1d21e7e96400d5d9f42ccce6552bb685a5"},
    {NULL, NULL},
  };

  int i;
  struct t *answer=NULL;
  char *program=argv[0];
  librdf_world *world;
  
  world=librdf_new_world();
  
  /* initialise digest module */
  librdf_init_digest(world);
  
  for(i=0; ((answer= &test_data_answers[i]) && answer->type != NULL) ; i++) {
    char *s;
    
    fprintf(stdout, "%s: Trying to create new %s digest\n", program, 
            answer->type);
    d=librdf_new_digest(world, answer->type);
    if(!d) {
      fprintf(stderr, "%s: Failed to create new digest type %s\n", program, 
              answer->type);
      continue;
    }
    fprintf(stdout, "%s: Initialising digest type %s\n", program, answer->type);
    librdf_digest_init(d);
                
    fprintf(stdout, "%s: Writing data into digest\n", program);
    librdf_digest_update(d, (unsigned char*)test_data, strlen(test_data));
                
    fprintf(stdout, "%s: Finishing digesting\n", program);
    librdf_digest_final(d);
    
    fprintf(stdout, "%s: %s digest of data is: ", program, answer->type);
    librdf_digest_print(d, stdout);
    fprintf(stdout, "\n");

    s=librdf_digest_to_string(d);
    if(strcmp(s, answer->result))
      fprintf(stderr, "%s: %s digest is wrong - expected: %s\n", program, answer->type, answer->result);
    else
      fprintf(stderr, "%s: %s digest is correct\n", program, answer->type);
    LIBRDF_FREE(cstring, s);
    
    fprintf(stdout, "%s: Freeing digest\n", program);
    librdf_free_digest(d);
  }
  
  
  /* finish digest module */
  librdf_finish_digest(world);

  LIBRDF_FREE(librdf_world, world);
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif
