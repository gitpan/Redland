/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_digest.h - RDF Digest Factory / Digest interfaces and definition
 *
 * $Id: rdf_digest.h,v 1.20 2001/02/20 20:16:32 cmdjb Exp $
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



#ifndef LIBRDF_DIGEST_H
#define LIBRDF_DIGEST_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef LIBRDF_INTERNAL

/* based on the GNUPG cipher/digest registration stuff */
struct librdf_digest_factory_s 
{
  struct librdf_digest_factory_s* next;
  char *   name;

  /* the rest of this structure is populated by the
     digest-specific register function */
  size_t  context_length;
  size_t  digest_length;

  /* functions (over context) */
  void (*init)( void *c );
  void (*update)( void *c, unsigned char *buf, size_t nbytes );
  void (*final)( void *c );
  unsigned char *(*get_digest)( void *c );
};


struct librdf_digest_s {
  char *context;
  unsigned char *digest;
  librdf_digest_factory* factory;
};

#endif


/* factory static methods */
void librdf_digest_register_factory(librdf_world *world, const char *name, void (*factory) (librdf_digest_factory*));

librdf_digest_factory* librdf_get_digest_factory(librdf_world *world, const char *name);


/* module init */
void librdf_init_digest(librdf_world *world);
/* module finish */
void librdf_finish_digest(librdf_world *world);
                    
/* constructor */
librdf_digest* librdf_new_digest(librdf_world *world, char *name);
librdf_digest* librdf_new_digest_from_factory(librdf_world *world, librdf_digest_factory *factory);

/* destructor */
void librdf_free_digest(librdf_digest *digest);


/* methods */
void librdf_digest_init(librdf_digest* digest);
void librdf_digest_update(librdf_digest* digest, unsigned char *buf, size_t length);
void librdf_digest_final(librdf_digest* digest);
void* librdf_digest_get_digest(librdf_digest* digest);

char* librdf_digest_to_string(librdf_digest* digest);
void librdf_digest_print(librdf_digest* digest, FILE* fh);


/* in librdf_digest_openssl.c */
#ifdef HAVE_OPENSSL_DIGESTS
void librdf_digest_openssl_constructor(librdf_world *world);
#endif

/* in librdf_digest_md5.c */
#ifdef HAVE_LOCAL_MD5_DIGEST
void librdf_digest_md5_constructor(librdf_world *world);
#endif

/* in librdf_digest_sha1.c */
#ifdef HAVE_LOCAL_SHA1_DIGEST
void librdf_digest_sha1_constructor(librdf_world *world);
#endif

/* in librdf_digest_ripemd160.c */
#ifdef HAVE_LOCAL_RIPEMD160_DIGEST
void librdf_digest_rmd160_constructor(librdf_world *world);
#endif


#ifdef __cplusplus
}
#endif

#endif
