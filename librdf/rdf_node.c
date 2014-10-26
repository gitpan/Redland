/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_node.c - RDF Node (Resource / Literal) Implementation
 *
 * $Id: rdf_node.c,v 1.82 2003/08/31 11:22:53 cmdjb Exp $
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

#include <librdf.h>
/* needed for utf8 functions and definition of 'byte' */
#include <rdf_utf8.h>


#ifndef STANDALONE

/* hashes - resource, literal, blank */
enum {
  H_RESOURCE,
  H_LITERAL,
  H_BLANK,
  H_LAST=H_BLANK
};

#define H_COUNT (H_LAST+1)


/* class functions */

/**
 * librdf_init_node - Initialise the node module.
 * @world: redland world object
 * 
 **/
void
librdf_init_node(librdf_world* world) 
{
  int i;
  
  for(i=0; i<H_COUNT; i++) {
    world->nodes_hash[i]=librdf_new_hash(world, NULL);
    if(!world->nodes_hash[i])
      LIBRDF_FATAL1(world, librdf_init_node,
                    "Failed to create Nodes hash from factory");
    
    if(librdf_hash_open(world->nodes_hash[i], NULL, 0, 1, 1, NULL))
      LIBRDF_FATAL1(world, librdf_init_node, "Failed to open Nodes hash");
  }
}


/**
 * librdf_finish_node - Terminate the librdf_node module
 * @world: redland world object
 **/
void
librdf_finish_node(librdf_world *world)
{
  int i;

  for(i=0; i<H_COUNT; i++) {
    if(world->nodes_hash[i]) {
      librdf_hash_close(world->nodes_hash[i]);
      librdf_free_hash(world->nodes_hash[i]);
    }
  }
}



/* constructors */

/**
 * librdf_new_node - Constructor - create a new librdf_node object with a private identifier
 * @world: redland world object
 * 
 * Calls librdf_new_node_from_blank_identifier(world, NULL) to
 * construct a new redland blank node identifier and make a
 * new librdf_node object for it.
 *
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node(librdf_world *world)
{
  return librdf_new_node_from_blank_identifier(world, (char*)NULL);
}

    

/*
 * librdf_new_node_from_uri_string_or_uri - INTERNAL - Constructor - create a new librdf_node object from a URI string or URI object
 * @world: redland world object
 * @uri_string: string representing a URI
 * @uri: &librdf_uri object
 * 
 * Return value: a new &librdf_node object or NULL on failure
 **/
static librdf_node*
librdf_new_node_from_uri_string_or_uri(librdf_world *world, 
                                       const char *uri_string, librdf_uri *uri) 
{
  librdf_node* new_node;
  librdf_uri *new_uri;
  librdf_hash_datum key, value; /* on stack - not allocated */
  librdf_hash_datum *old_value;

  if(!uri_string && !uri)
    return NULL;

  if(uri_string && uri) {
    LIBRDF_DEBUG3(librdf_new_node_from_uri_or_uri, "Called with both a URI string %s and object URI %s\n", uri_string, librdf_uri_as_string(uri));
    return NULL;
  }

  if(uri_string) {
    new_uri=librdf_new_uri(world, uri_string);
    if(!new_uri)
      return NULL;
  } else
    new_uri=librdf_new_uri_from_uri(uri);
  

#ifdef WITH_THREADS
  pthread_mutex_lock(world->nodes_mutex);
#endif
  
  key.data=&new_uri;
  key.size=sizeof(librdf_uri*);

  /* if the existing node found in resource hash, return it */
  if((old_value=librdf_hash_get_one(world->nodes_hash[H_RESOURCE], &key))) {
    new_node=*(librdf_node**)old_value->data;

    librdf_free_uri(new_uri);
    
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
    LIBRDF_DEBUG3(librdf_new_node, "Found existing resource node with URI %s in hash with current usage %d\n", uri_string, new_node->usage);
#endif

    librdf_free_hash_datum(old_value);
    new_node->usage++;

    goto unlock;
  }


  /* otherwise create a new one */

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG2(librdf_new_node, "Creating new resource node with URI %s in hash\n", uri_string);
#endif

  new_node = (librdf_node*)LIBRDF_CALLOC(librdf_node, 1, sizeof(librdf_node));
  if(!new_node) {
    librdf_free_uri(new_uri);
    goto unlock;
  }
  
  new_node->world=world;
  new_node->value.resource.uri=new_uri;
  new_node->type = LIBRDF_NODE_TYPE_RESOURCE;

  new_node->usage=1;

  value.data=&new_node; value.size=sizeof(librdf_node*);

  /* store in hash: (librdf_uri*)uri => (librdf_node*) */
  if(librdf_hash_put(world->nodes_hash[H_RESOURCE], &key, &value)) {
    LIBRDF_FREE(librdf_node, new_node);
    librdf_free_uri(new_uri);
    new_node=NULL;
  }

  
 unlock:
#ifdef WITH_THREADS
  pthread_mutex_unlock(world->nodes_mutex);
#endif

  return new_node;
}

    

/**
 * librdf_new_node_from_uri_string - Constructor - create a new librdf_node object from a URI string
 * @world: redland world object
 * @uri_string: string representing a URI
 * 
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_uri_string(librdf_world *world, const char *uri_string) 
{
  return librdf_new_node_from_uri_string_or_uri(world, uri_string, NULL);
}

    

/* Create a new (Resource) Node and set the URI. */

/**
 * librdf_new_node_from_uri - Constructor - create a new resource librdf_node object with a given URI
 * @world: redland world object
 * @uri: &rdf_uri object
 *
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_uri(librdf_world *world, librdf_uri *uri) 
{
  return librdf_new_node_from_uri_string_or_uri(world, NULL, uri);
}


/**
 * librdf_new_node_from_uri_local_name - Constructor - create a new resource librdf_node object with a given URI and local name
 * @world: redland world object
 * @uri: &rdf_uri object
 * @local_name: local name to append to URI
 *
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_uri_local_name(librdf_world *world, 
                                    librdf_uri *uri, const char *local_name) 
{
  librdf_uri *new_uri;
  librdf_node* new_node;

  new_uri=librdf_new_uri_from_uri_local_name(uri, local_name);
  if(!new_uri)
    return NULL;

  new_node=librdf_new_node_from_uri_string_or_uri(world, NULL, new_uri);

  librdf_free_uri(new_uri);

  return new_node;
}


/**
 * librdf_new_node_from_normalised_uri_string - Constructor - create a new librdf_node object from a URI string normalised to a new base URI
 * @world: redland world object
 * @uri_string: string representing a URI
 * @source_uri: source URI
 * @base_uri: base URI
 * 
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_normalised_uri_string(librdf_world *world, 
                                           const char *uri_string,
                                           librdf_uri *source_uri,
                                           librdf_uri *base_uri)
{
  librdf_uri* new_uri;
  librdf_node* new_node;
  
  new_uri=librdf_new_uri_normalised_to_base(uri_string, source_uri, base_uri);
  if(!new_uri)
    return NULL;

  new_node=librdf_new_node_from_uri_string_or_uri(world, NULL, new_uri);
  librdf_free_uri(new_uri);
  
  return new_node;
}


/**
 * librdf_new_node_from_literal -  Constructor - create a new literal librdf_node object
 * @world: redland world object
 * @string: literal string value
 * @xml_language: literal XML language (or NULL, empty string)
 * @is_wf_xml: non 0 if literal is XML
 * 
 * 0.9.12: xml_space argument deleted
 *
 * Return value: new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_literal(librdf_world *world, 
                             const char *string, const char *xml_language, 
                             int is_wf_xml) 
{
  return librdf_new_node_from_typed_literal(world,
                                            string, xml_language,
                                            (is_wf_xml ? 
                                             LIBRDF_RS_XMLLiteral_URI : NULL)
                                            );
}


/**
 * librdf_new_node_from_typed_literal -  Constructor - create a new typed literal librdf_node object
 * @world: redland world object
 * @string: literal string value
 * @xml_language: literal XML language (or NULL, empty string)
 * @datatype_uri: URI of typed literal datatype or NULL
 * 
 * Return value: new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_typed_literal(librdf_world *world, 
                                   const char *value,
                                   const char *xml_language, 
                                   librdf_uri* datatype_uri) 
{
  librdf_node* new_node;
  int value_len;
  char *new_value;
  char *new_xml_language;
  librdf_hash_datum key, value_hd; /* on stack - not allocated */
  librdf_hash_datum *old_value;
  size_t size;
  unsigned char *buffer;
  
#ifdef WITH_THREADS
  pthread_mutex_lock(world->nodes_mutex);
#endif

  new_node = (librdf_node*)LIBRDF_CALLOC(librdf_node, 1, sizeof(librdf_node));
  if(!new_node)
    goto unlock;

  new_node->world=world;
  
  /* set type */
  new_node->type=LIBRDF_NODE_TYPE_LITERAL;

  /* the only time the string literal length should ever be measured */
  new_node->value.literal.string_len = value_len = strlen(value);
  
  new_value=(char*)LIBRDF_MALLOC(cstring, value_len + 1);
  if(!new_value) {
    LIBRDF_FREE(librdf_node, new_node);
    new_node=NULL;
    goto unlock;
  }
  strcpy(new_value, value);
  new_node->value.literal.string=(char*)new_value;
  
  if(xml_language && *xml_language) {
    new_xml_language=(char*)LIBRDF_MALLOC(cstring, strlen(xml_language) + 1);
    if(!new_xml_language) {
      LIBRDF_FREE(cstring, new_value);
      LIBRDF_FREE(librdf_node, new_node);
      new_node=NULL;
      goto unlock;
    }
    strcpy(new_xml_language, xml_language);
    new_node->value.literal.xml_language=new_xml_language;
  } else
    new_xml_language=NULL;
  
  if(datatype_uri) {
    datatype_uri=librdf_new_uri_from_uri(datatype_uri);
    new_node->value.literal.datatype_uri=datatype_uri;
  }
  

  size=librdf_node_encode(new_node, NULL, 0);
  if(size)
    buffer=(char*)LIBRDF_MALLOC(cstring, size);
  else
    buffer=NULL;
  
  if(!buffer) {
    if(datatype_uri)
      librdf_free_uri(datatype_uri);
    LIBRDF_FREE(cstring, new_value);
    LIBRDF_FREE(librdf_node, new_node);
    return NULL;
  }

  new_node->value.literal.size=size;
  new_node->value.literal.key=buffer;
  librdf_node_encode(new_node, buffer, size);

  key.data=buffer;
  key.size=size;

  /* if the existing node found in resource hash, return it */
  if((old_value=librdf_hash_get_one(world->nodes_hash[H_LITERAL], &key))) {
    LIBRDF_FREE(cstring, buffer);
    if(new_xml_language)
      LIBRDF_FREE(cstring, new_xml_language);
    if(datatype_uri)
      librdf_free_uri(datatype_uri);
    LIBRDF_FREE(cstring, new_value);
    LIBRDF_FREE(librdf_node, new_node);

    new_node=*(librdf_node**)old_value->data;

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
    LIBRDF_DEBUG3(librdf_new_node_from_typed_literal, "Found existing resource node with typed literal %s in hash with current usage %d\n", value, new_node->usage);
#endif

    librdf_free_hash_datum(old_value);
    new_node->usage++;

    goto unlock;
  }
    
  /* otherwise add the new node */
  new_node->usage=1;

  value_hd.data=&new_node; value_hd.size=sizeof(librdf_node*);

  /* store in hash: (serialised node) => (librdf_node*) */
  if(librdf_hash_put(world->nodes_hash[H_LITERAL], &key, &value_hd)) {
    LIBRDF_FREE(cstring, buffer);
    if(new_xml_language)
      LIBRDF_FREE(cstring, new_xml_language);
    if(datatype_uri)
      librdf_free_uri(datatype_uri);
    LIBRDF_FREE(cstring, new_value);
    LIBRDF_FREE(librdf_node, new_node);

    new_node=NULL;
  }

  
 unlock:
#ifdef WITH_THREADS
    pthread_mutex_unlock(world->nodes_mutex);
#endif

  return new_node;
}


/**
 * librdf_new_node_from_blank_identifier -  Constructor - create a new literal librdf_node object from a blank node identifier
 * @world: redland world object
 * @identifier: blank node identifier or NULL
 *
 * If no identifier string is given, creates a new internal identifier
 * and assigns it.
 * 
 * Return value: new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_blank_identifier(librdf_world *world,
                                      const char *identifier)
{
  librdf_node* new_node;
  char *new_identifier;
  int len;
  librdf_hash_datum key, value; /* on stack - not allocated */
  librdf_hash_datum *old_value;

#ifdef WITH_THREADS
  pthread_mutex_lock(world->nodes_mutex);
#endif

  if(!identifier)
    identifier=librdf_world_get_genid(world);

  len=strlen(identifier);
  
  new_identifier=(char*)LIBRDF_MALLOC(cstring, len+1);
  if(!new_identifier) {
    new_node=NULL;
    goto unlock;
  }
  strcpy(new_identifier, identifier);


  key.data=new_identifier;
  key.size=len;

  /* if the existing node found in resource hash, return it */
  if((old_value=librdf_hash_get_one(world->nodes_hash[H_BLANK], &key))) {
    new_node=*(librdf_node**)old_value->data;

    LIBRDF_FREE(cstring, new_identifier);
    
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
    LIBRDF_DEBUG3(librdf_new_node_from_blank_identifier, "Found existing blank node identifier %s in hash with current usage %d\n", new_identifier, new_node->usage);
#endif

    librdf_free_hash_datum(old_value);
    new_node->usage++;

    goto unlock;
  }


  new_node = (librdf_node*)LIBRDF_CALLOC(librdf_node, 1, sizeof(librdf_node));
  if(!new_node) {
    LIBRDF_FREE(cstring, new_identifier);
    goto unlock;
  }

  new_node->world=world;
  new_node->value.blank.identifier=new_identifier;
  new_node->value.blank.identifier_len=len;
  new_node->type=LIBRDF_NODE_TYPE_BLANK;

  new_node->usage=1;

  value.data=&new_node; value.size=sizeof(librdf_node*);

  /* store in hash: (blank node ID string) => (librdf_node*) */
  if(librdf_hash_put(world->nodes_hash[H_BLANK], &key, &value)) {
    LIBRDF_FREE(librdf_node, new_node);
    LIBRDF_FREE(cstring, new_identifier);
    new_node=NULL;
  }


 unlock:
#ifdef WITH_THREADS
    pthread_mutex_unlock(world->nodes_mutex);
#endif

  return new_node;
}


/**
 * librdf_new_node_from_node - Copy constructor - create a new librdf_node object from an existing librdf_node object
 * @node: &librdf_node object to copy
 * 
 * Return value: a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_new_node_from_node(librdf_node *node) 
{
  node->usage++;
  return node;
}


/**
 * librdf_free_node - Destructor - destroy an librdf_node object
 * @node: &librdf_node object
 * 
 **/
void
librdf_free_node(librdf_node *node) 
{
  librdf_hash_datum key; /* on stack */
#ifdef WITH_THREADS
  librdf_world *world = node->world;
#endif

#ifdef WITH_THREADS
  pthread_mutex_lock(world->nodes_mutex);
#endif

  node->usage--;
  
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG3(librdf_free_node, "Node %s usage count now %d\n", node->string, node->usage);
#endif

  /* decrement usage, don't free if not 0 yet*/
  if(node->usage) {
#ifdef WITH_THREADS
    pthread_mutex_unlock(world->nodes_mutex);
#endif
    return;
  }

#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG3(librdf_free_node, "Deleting NODE %s from hash, max usage was %d\n", node->string, node->max_usage);
#endif

  switch(node->type) {
    case LIBRDF_NODE_TYPE_RESOURCE:
      key.data=&node->value.resource.uri;
      key.size=sizeof(librdf_uri*);
      if(librdf_hash_delete_all(node->world->nodes_hash[H_RESOURCE], &key) )
        LIBRDF_FATAL1(world, librdf_free_node, "Hash deletion failed");

      librdf_free_uri(node->value.resource.uri);
      break;
      
    case LIBRDF_NODE_TYPE_LITERAL:
      if(node->value.literal.key) {
        key.data=node->value.literal.key;
        key.size=node->value.literal.size;
        if(librdf_hash_delete_all(node->world->nodes_hash[H_LITERAL], &key) )
          LIBRDF_FATAL1(world, librdf_free_node, "Hash deletion failed");
        LIBRDF_FREE(cstring, node->value.literal.key);
      }
      
      if(node->value.literal.string != NULL)
        LIBRDF_FREE(cstring, node->value.literal.string);
      if(node->value.literal.xml_language != NULL)
        LIBRDF_FREE(cstring, node->value.literal.xml_language);
      if(node->value.literal.datatype_uri != NULL)
        librdf_free_uri(node->value.literal.datatype_uri);
      break;

    case LIBRDF_NODE_TYPE_BLANK:
      key.data=node->value.blank.identifier;
      key.size=node->value.blank.identifier_len;
      if(librdf_hash_delete_all(node->world->nodes_hash[H_BLANK], &key) )
        LIBRDF_FATAL1(world, librdf_free_node, "Hash deletion failed");

      if(node->value.blank.identifier != NULL)
        LIBRDF_FREE(cstring, node->value.blank.identifier);
      break;
      
    default:
      break;
  }

#ifdef WITH_THREADS
  pthread_mutex_unlock(world->nodes_mutex);
#endif

  LIBRDF_FREE(librdf_node, node);
}


/* functions / methods */

/**
 * librdf_node_get_uri - Get the URI for a node object
 * @node: the node object
 *
 * Returns a pointer to the URI object held by the node, it must be
 * copied if it is wanted to be used by the caller.
 * 
 * Return value: URI object or NULL if node has no URI.
 **/
librdf_uri*
librdf_node_get_uri(librdf_node* node) 
{
  if(node->type != LIBRDF_NODE_TYPE_RESOURCE)
    return NULL;
  
  return node->value.resource.uri;
}


/**
 * librdf_node_get_type - Get the type of the node
 * @node: the node object
 * 
 * Return value: the node type
 **/
librdf_node_type
librdf_node_get_type(librdf_node* node) 
{
  return node->type;
}


#ifdef LIBRDF_DEBUG
/* FIXME: For debugging purposes only */
static const char* const librdf_node_type_names[] =
{"Unknown", "Resource", "Literal", "<Reserved1>", "Blank"};


/*
 * librdf_node_get_type_as_string - Get a string representation for the type of the node
 * @type: the node type 
 * 
 * The type is that returned by the librdf_node_get_type method
 *
 * Return value: a pointer to a shared copy of the string or NULL if unknown.
 **/
const char*
librdf_node_get_type_as_string(int type)
{
  if(type < 0 || type > LIBRDF_NODE_TYPE_LAST)
    return NULL;
  return librdf_node_type_names[type];
}
#endif





/**
 * librdf_node_get_literal_value - Get the string literal value of the node
 * @node: the node object
 * 
 * Returns a pointer to the literal value held by the node, it must be
 * copied if it is wanted to be used by the caller.
 *
 * Return value: the literal string or NULL if node is not a literal
 **/
char*
librdf_node_get_literal_value(librdf_node* node) 
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return NULL;
  return node->value.literal.string;
}


/**
 * librdf_node_get_literal_value_as_counted_string - Get the string literal value of the node as a counted string
 * @node: the node object
 * @len_p: pointer to location to store length (or NULL)
 * 
 * Returns a pointer to the literal value held by the node, it must be
 * copied if it is wanted to be used by the caller.
 *
 * Return value: the literal string or NULL if node is not a literal
 **/
char*
librdf_node_get_literal_value_as_counted_string(librdf_node* node, 
                                                size_t *len_p) 
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return NULL;
  if(len_p)
    *len_p=node->value.literal.string_len;
  return node->value.literal.string;
}


/**
 * librdf_node_get_literal_value_as_latin1 - Get the string literal value of the node as ISO Latin-1
 * @node: the node object
 * 
 * Returns a newly allocated string containing the conversion of the
 * UTF-8 literal value held by the node.
 *
 * Return value: the literal string or NULL if node is not a literal
 **/
char*
librdf_node_get_literal_value_as_latin1(librdf_node* node) 
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return NULL;
  return (char*)librdf_utf8_to_latin1((const byte*)node->value.literal.string,
                                      node->value.literal.string_len, NULL);
}


/**
 * librdf_node_get_literal_value_language - Get the XML language of the node
 * @node: the node object
 * 
 * Returns a pointer to the literal language value held by the node, it must
 * be copied if it is wanted to be used by the caller.
 *
 * Return value: the XML language string or NULL if node is not a literal
 * or there is no XML language defined.
 **/
char*
librdf_node_get_literal_value_language(librdf_node* node) 
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return NULL;
  return node->value.literal.xml_language;
}


/**
 * librdf_node_get_literal_value_is_wf_xml - Get the XML well-formness property of the node
 * @node: the node object
 * 
 * Return value: 0 if the XML literal is NOT well formed XML content, or the node is not a literal
 **/
int
librdf_node_get_literal_value_is_wf_xml(librdf_node* node) 
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return 0;
  return librdf_uri_equals(node->value.literal.datatype_uri,
                           LIBRDF_RS_XMLLiteral_URI);
}


/**
 * librdf_node_get_literal_value_datatype_uri - Get the typed literal datatype URI of the literal node
 * @node: the node object
 * 
 * Return value: shared URI of the datatyped literal or NULL if the node is not a literal, or has no datatype URI
 **/
librdf_uri*
librdf_node_get_literal_value_datatype_uri(librdf_node* node)
{
  if(node->type != LIBRDF_NODE_TYPE_LITERAL)
    return 0;
  return node->value.literal.datatype_uri;
}


/**
 * librdf_node_get_li_ordinal - Get the node li object ordinal value
 * @node: the node object
 *
 * Return value: the li ordinal value or < 1 on failure
 **/
int
librdf_node_get_li_ordinal(librdf_node* node) {
  char *uri_string;
  
  if(node->type != LIBRDF_NODE_TYPE_RESOURCE);
    return -1;

  uri_string=librdf_uri_as_string(node->value.resource.uri); 
  if(strncmp(uri_string, "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44))
    return -1;
  
  return atoi(uri_string+44);
}


/**
 * librdf_node_get_blank_identifier - Get the blank node identifier
 * @node: the node object
 *
 * Return value: the identifier value
 **/
char *
librdf_node_get_blank_identifier(librdf_node* node) {
  return node->value.blank.identifier;
}


/**
 * librdf_node_is_resource -  Check node is a resource
 * @node: the node object
 * 
 * Return value: non-zero if the node is a resource (URI)
 **/
int
librdf_node_is_resource(librdf_node* node) {
  return (node->type == LIBRDF_NODE_TYPE_RESOURCE);
}


/**
 * librdf_node_is_resource -  Check node is a literal
 * @node: the node object
 * 
 * Return value: non-zero if the node is a literal
 **/
int
librdf_node_is_literal(librdf_node* node) {
  return (node->type == LIBRDF_NODE_TYPE_LITERAL);
}


/**
 * librdf_node_is_blank -  Check node is a blank nodeID
 * @node: the node object
 * 
 * Return value: non-zero if the node is a blank nodeID
 **/
int
librdf_node_is_blank(librdf_node* node) {
  return (node->type == LIBRDF_NODE_TYPE_BLANK);
}


/**
 * librdf_node_to_string - Format the node as a string
 * @node: the node object
 * 
 * Note a new string is allocated which must be freed by the caller.
 * 
 * Return value: a string value representing the node or NULL on failure
 **/
char*
librdf_node_to_string(librdf_node* node) 
{
  return librdf_node_to_counted_string(node, NULL);
}


/**
 * librdf_node_to_counted_string - Format the node as a counted string
 * @node: the node object
 * @len_p: pointer to location to store length
 * 
 * Note a new string is allocated which must be freed by the caller.
 * 
 * Return value: a string value representing the node or NULL on failure
 **/
char*
librdf_node_to_counted_string(librdf_node* node, size_t* len_p) 
{
  char *uri_string;
  size_t len;
  char *s;

  switch(node->type) {
  case LIBRDF_NODE_TYPE_RESOURCE:
    uri_string=librdf_uri_to_counted_string(node->value.resource.uri, &len);
    if(!uri_string)
      return NULL;
    len +=2;
    if(len_p)
      *len_p=len;
    s=(char*)LIBRDF_MALLOC(cstring, len+1);
    if(!s) {
      LIBRDF_FREE(cstring, uri_string);
      return NULL;
    }
    sprintf(s, "[%s]", uri_string);
    LIBRDF_FREE(cstring, uri_string);
    break;
  case LIBRDF_NODE_TYPE_LITERAL:
    len=node->value.literal.string_len;
    if(len_p)
      *len_p=len;
    s=(char*)LIBRDF_MALLOC(cstring, len+1);
    if(!s)
      return NULL;
    /* use strcpy here to add \0 to end of literal string */
    strcpy(s, node->value.literal.string);
    break;
  case LIBRDF_NODE_TYPE_BLANK:
    len=node->value.blank.identifier_len + 2;
    if(len_p)
      *len_p=len;
    s=(char*)LIBRDF_MALLOC(cstring, len+1);
    if(!s)
      return NULL;
    sprintf(s, "(%s)", node->value.blank.identifier);
    break;
  default:
    LIBRDF_ERROR2(node->world, librdf_node_string, 
                  "Do not know how to print node type %d\n", node->type);
    return NULL;
  }
  return s;
}


/**
 * librdf_node_print - pretty print the node to a file descriptor
 * @node: the node
 * @fh: file handle
 * 
 * This method is for debugging and the format of the output should
 * not be relied on.
 * 
 **/
void
librdf_node_print(librdf_node* node, FILE *fh)
{
  char* s;

  if(!node)
    return;
  
  s=librdf_node_to_string(node);
  if(!s)
    return;
  fputs(s, fh);
  LIBRDF_FREE(cstring, s);
}



/**
 * librdf_node_get_digest - Get a digest representing a librdf_node
 * @node: the node object
 * 
 * A new digest object is created which must be freed by the caller.
 * 
 * Return value: a new &librdf_digest object or NULL on failure
 **/
librdf_digest*
librdf_node_get_digest(librdf_node* node) 
{
  librdf_digest* d=NULL;
  char *s;
  librdf_world* world=node->world;
  
  switch(node->type) {
    case LIBRDF_NODE_TYPE_RESOURCE:
      d=librdf_uri_get_digest(node->value.resource.uri);
      break;
      
    case LIBRDF_NODE_TYPE_LITERAL:
      s=node->value.literal.string;
      d=librdf_new_digest_from_factory(world, world->digest_factory);
      if(!d)
        return NULL;
      
      librdf_digest_init(d);
      librdf_digest_update(d, (unsigned char*)s, node->value.literal.string_len);
      librdf_digest_final(d);
      break;
    default:
      LIBRDF_ERROR2(world, librdf_node_get_digest,
                    "Do not know how to make digest for node type %d\n", 
                    node->type);
      return NULL;
  }
  
  return d;
}


/**
 * librdf_node_equals - Compare two librdf_node objects for equality
 * @first_node: first &librdf_node node
 * @second_node: second &librdf_node node
 * 
 * Note - for literal nodes, XML language, XML space and well-formness are 
 * presently ignored in the comparison.
 * 
 * Return value: non 0 if nodes are equal.  0 if not-equal or failure
 **/
int
librdf_node_equals(librdf_node* first_node, librdf_node* second_node) 
{
  if(!first_node || !second_node)
    return 0;
  
  return (first_node == second_node);
}



/**
 * librdf_node_encode - Serialise a node into a buffer
 * @node: the node to serialise
 * @buffer: the buffer to use
 * @length: buffer size
 * 
 * Encodes the given node in the buffer, which must be of sufficient
 * size.  If buffer is NULL, no work is done but the size of buffer
 * required is returned.
 * 
 * Return value: the number of bytes written or 0 on failure.
 **/
size_t
librdf_node_encode(librdf_node* node, unsigned char *buffer, size_t length)
{
  size_t total_length=0;
  unsigned char *string;
  size_t string_length;
  size_t language_length=0;
  unsigned char *datatype_uri_string;
  size_t datatype_uri_length=0;
  
  switch(node->type) {
    case LIBRDF_NODE_TYPE_RESOURCE:
      string=(unsigned char*)librdf_uri_as_counted_string(node->value.resource.uri, &string_length);
      
      total_length= 3 + string_length + 1; /* +1 for \0 at end */
      
      if(length && total_length > length)
        return 0;    
      
      if(buffer) {
        buffer[0]='R';
        buffer[1]=(string_length & 0xff00) >> 8;
        buffer[2]=(string_length & 0x00ff);
        strcpy((char*)buffer+3, (char*)string);
      }
      break;
      
    case LIBRDF_NODE_TYPE_LITERAL:
#if 0
      /* old encoding before 0.9.12 */
      string=(unsigned char*)node->value.literal.string;
      string_length=node->value.literal.string_len;
      if(node->value.literal.xml_language)
        language_length=strlen(node->value.literal.xml_language);
      
      total_length= 6 + string_length + 1; /* +1 for \0 at end */
      if(language_length)
        total_length += language_length+1;
      
      if(length && total_length > length)
        return 0;    
      
      if(buffer) {
        buffer[0]='L';
        buffer[1]=(node->value.literal.is_wf_xml & 0xf) << 8;
        buffer[2]=(string_length & 0xff00) >> 8;
        buffer[3]=(string_length & 0x00ff);
        buffer[4]='\0'; /* unused */
        buffer[5]=(language_length & 0x00ff);
        strcpy((char*)buffer+6, (const char*)string);
        if(language_length)
          strcpy((char*)buffer+string_length+7, (const char*)node->value.literal.xml_language);
      }
      break;
      
#endif
      string=(unsigned char*)node->value.literal.string;
      string_length=node->value.literal.string_len;
      if(node->value.literal.xml_language)
        language_length=strlen(node->value.literal.xml_language);
      if(node->value.literal.datatype_uri) {
        datatype_uri_string=librdf_uri_as_counted_string(node->value.literal.datatype_uri, &datatype_uri_length);
      }
      
      total_length= 6 + string_length + 1; /* +1 for \0 at end */
      if(language_length)
        total_length += language_length+1;
      if(datatype_uri_length)
        total_length += datatype_uri_length+1;
      
      if(length && total_length > length)
        return 0;    
      
      if(buffer) {
        buffer[0]='M';
        buffer[1]=(string_length & 0xff00) >> 8;
        buffer[2]=(string_length & 0x00ff);
        buffer[3]=(datatype_uri_length & 0xff00) >> 8;
        buffer[4]=(datatype_uri_length & 0x00ff);
        buffer[5]=(language_length & 0x00ff);
        buffer += 6;
        strcpy((char*)buffer, (const char*)string);
        buffer += string_length+1;
        if(datatype_uri_length) {
          strcpy((char*)buffer, datatype_uri_string);
          buffer += datatype_uri_length+1;
        }
        if(language_length)
          strcpy((char*)buffer, (const char*)node->value.literal.xml_language);
      }
      break;
      
    case LIBRDF_NODE_TYPE_BLANK:
      string=(unsigned char*)node->value.blank.identifier;
      string_length=node->value.blank.identifier_len;
      
      total_length= 3 + string_length + 1; /* +1 for \0 at end */
      
      if(length && total_length > length)
        return 0;    
      
      if(buffer) {
        buffer[0]='B';
        buffer[1]=(string_length & 0xff00) >> 8;
        buffer[2]=(string_length & 0x00ff);
        strcpy((char*)buffer+3, (const char*)string);
      }
      break;
      
    default:
      LIBRDF_ERROR2(node->world, librdf_node_encode,
                    "Do not know how to encode node type %d\n", node->type);
      return 0;
  }
  
  return total_length;
}


/**
 * librdf_node_decode - Deserialise a node from a buffer
 * @world: librdf_world
 * @size_p: pointer to bytes used or NULL
 * @buffer: the buffer to use
 * @length: buffer size
 * 
 * Decodes the serialised node (as created by librdf_node_encode() )
 * from the given buffer.
 * 
 * Return value: new node or NULL on failure (bad encoding, allocation failure)
 **/
librdf_node*
librdf_node_decode(librdf_world *world,
                   size_t* size_p, unsigned char *buffer, size_t length)
{
  int is_wf_xml;
  size_t string_length;
  size_t total_length;
  size_t language_length;
  char *datatype_uri_string=NULL;
  size_t datatype_uri_length;
  librdf_uri* datatype_uri=NULL;
  char *language=NULL;
  int status=0;
  librdf_node* node=NULL;

  /* absolute minimum - first byte is type */
  if (length < 1)
    return NULL;

  total_length=0;
  switch(buffer[0]) {
    case 'R': /* LIBRDF_NODE_TYPE_RESOURCE */
      /* min */
      if(length < 3)
        return NULL;

      string_length=(buffer[1] << 8) | buffer[2];
      total_length = 3 + string_length + 1;
      
      node = librdf_new_node_from_uri_string(world, (const char*)buffer+3);

      break;

    case 'L': /* Old encoding form for LIBRDF_NODE_TYPE_LITERAL */
      /* min */
      if(length < 6)
        return NULL;
      
      is_wf_xml=(buffer[1] & 0xf0)>>8;
      string_length=(buffer[2] << 8) | buffer[3];
      language_length=buffer[5];

      total_length= 6 + string_length + 1; /* +1 for \0 at end */
      if(language_length) {
        language = buffer + total_length;
        total_length += language_length+1;
      }
      
      node=librdf_new_node_from_typed_literal(world,
                                              (const char*)buffer+6,
                                              (const char*)language,
                                              is_wf_xml ? LIBRDF_RS_XMLLiteral_URI : NULL);
    
    break;

    case 'M': /* LIBRDF_NODE_TYPE_LITERAL 0.9.12+ */
      /* min */
      if(length < 6)
        return NULL;
      
      string_length=(buffer[1] << 8) | buffer[2];
      datatype_uri_length=(buffer[3] << 8) | buffer[4];
      language_length=buffer[5];

      total_length= 6 + string_length + 1; /* +1 for \0 at end */
      if(datatype_uri_length) {
        datatype_uri_string = buffer + total_length;
        total_length += datatype_uri_length+1;
      }
      if(language_length) {
        language = buffer + total_length;
        total_length += language_length+1;
      }

      if(datatype_uri_string)
        datatype_uri=librdf_new_uri(world, datatype_uri_string);
      
      node=librdf_new_node_from_typed_literal(world,
                                              (const char*)buffer+6,
                                              (const char*)language,
                                              datatype_uri);
      if(datatype_uri)
        librdf_free_uri(datatype_uri);
      
      if(status)
        return NULL;
      
    break;

    case 'B': /* LIBRDF_NODE_TYPE_BLANK */
      /* min */
      if(length < 3)
        return NULL;
      
      string_length=(buffer[1] << 8) | buffer[2];

      total_length= 3 + string_length + 1; /* +1 for \0 at end */
      
      node = librdf_new_node_from_blank_identifier(world, (const char*)buffer+3);
    
    break;

  default:
    LIBRDF_ERROR2(world, librdf_node_decode,
                  "Illegal node encoding '%c' seen\n", buffer[0]);
    return NULL;
  }
  
  if(size_p)
    *size_p=total_length;

  return node;
}



/* iterator over a static array of nodes; - mostly for testing */
static int librdf_node_static_iterator_is_end(void* iterator);
static int librdf_node_static_iterator_next_method(void* iterator);
static void* librdf_node_static_iterator_get_method(void* iterator, int flags);
static void librdf_node_static_iterator_finished(void* iterator);

typedef struct {
  librdf_world *world;
  librdf_node** nodes; /* static array of nodes; shared */
  int size;            /* size of above array */
  int current;         /* index into above array */
} librdf_node_static_iterator_context;


static int
librdf_node_static_iterator_is_end(void* iterator)
{
  librdf_node_static_iterator_context* context=(librdf_node_static_iterator_context*)iterator;

  return (context->current > context->size-1);
}


static int
librdf_node_static_iterator_next_method(void* iterator) 
{
  librdf_node_static_iterator_context* context=(librdf_node_static_iterator_context*)iterator;

  if(context->current > context->size-1)
    return 1;

  context->current++;
  return 0;
}


static void*
librdf_node_static_iterator_get_method(void* iterator, int flags) 
{
  librdf_node_static_iterator_context* context=(librdf_node_static_iterator_context*)iterator;
  
  if(context->current > context->size-1)
    return NULL;

  switch(flags) {
    case LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT:
       return (void*)context->nodes[context->current];

    case LIBRDF_ITERATOR_GET_METHOD_GET_CONTEXT:
      return NULL;

    default:
      LIBRDF_ERROR2(context->world, librdf_node_static_iterator_get_method,
                    "Unknown iterator method flag %d\n", flags);
      return NULL;
  }
}


static void
librdf_node_static_iterator_finished(void* iterator) 
{
  librdf_node_static_iterator_context* context=(librdf_node_static_iterator_context*)iterator;
  LIBRDF_FREE(librdf_node_static_iterator_context, context);
}


/**
 * librdf_node_static_iterator_create - Create an iterator over an array of nodes
 * @nodes: static array of &librdf_node objects
 * @size: size of array
 * 
 * This creates an iterator for an existing static array of librdf_node
 * objects.  It is mostly intended for testing iterator code.
 * 
 * Return value: a &librdf_iterator serialization of the nodes or NULL on failure
 **/
librdf_iterator*
librdf_node_static_iterator_create(librdf_node** nodes,
                                   int size)
{
  librdf_node_static_iterator_context* context;
  librdf_iterator *iterator;
  
  context=(librdf_node_static_iterator_context*)LIBRDF_CALLOC(librdf_node_static_iterator_context, 1, sizeof(librdf_node_static_iterator_context));
  if(!context)
    return NULL;

  context->nodes=nodes;
  context->size=size;
  context->current=0;

  iterator=librdf_new_iterator(nodes[0]->world,
                               (void*)context,
                               librdf_node_static_iterator_is_end,
                               librdf_node_static_iterator_next_method,
                               librdf_node_static_iterator_get_method,
                               librdf_node_static_iterator_finished);
  if(!iterator)
    librdf_node_static_iterator_finished(context);

  return iterator;
}

#endif


/* TEST CODE */


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  librdf_node *node, *node2, *node3, *node4, *node5, *node6, *node7;
  char *hp_string1="http://www.ilrt.bristol.ac.uk/people/cmdjb/";
  char *hp_string2="http://purl.org/net/dajobe/";
  char *lit_string="Dave Beckett";
  char *genid="genid42";
  librdf_uri *uri, *uri2;
  int size, size2;
  char *buffer;
  librdf_world *world;
  
  char *program=argv[0];
	
  world=librdf_new_world();
  librdf_world_init_mutex(world);

  librdf_init_digest(world);
  librdf_init_hash(world);
  librdf_init_uri(world);
  librdf_init_node(world);

  fprintf(stdout, "%s: Creating home page node from string\n", program);
  node=librdf_new_node_from_uri_string(world, hp_string1);
  if(!node) {
    fprintf(stderr, "%s: librdf_new_node_from_uri_string failed\n", program);
    return(1);
  }
  
  fprintf(stdout, "%s: Home page URI is ", program);
  librdf_uri_print(librdf_node_get_uri(node), stdout);
  fputc('\n', stdout);
  
  fprintf(stdout, "%s: Creating URI from string '%s'\n", program, 
          hp_string2);
  uri=librdf_new_uri(world, hp_string2);
  fprintf(stdout, "%s: Setting node URI to new URI ", program);
  librdf_uri_print(uri, stdout);
  fputc('\n', stdout);
  librdf_free_uri(uri);
  
  fprintf(stdout, "%s: Node is: ", program);
  librdf_node_print(node, stdout);
  fputc('\n', stdout);

  size=librdf_node_encode(node, NULL, 0);
  fprintf(stdout, "%s: Encoding node requires %d bytes\n", program, size);
  buffer=(char*)LIBRDF_MALLOC(cstring, size);

  fprintf(stdout, "%s: Encoding node in buffer\n", program);
  size2=librdf_node_encode(node, (unsigned char*)buffer, size);
  if(size2 != size) {
    fprintf(stderr, "%s: Encoding node used %d bytes, expected it to use %d\n", program, size2, size);
    return(1);
  }
  
    
  fprintf(stdout, "%s: Creating new node\n", program);

  fprintf(stdout, "%s: Decoding node from buffer\n", program);
  if(!(node2=librdf_node_decode(world, NULL, (unsigned char*)buffer, size))) {
    fprintf(stderr, "%s: Decoding node failed\n", program);
    return(1);
  }
  LIBRDF_FREE(cstring, buffer);
   
  fprintf(stdout, "%s: New node is: ", program);
  librdf_node_print(node2, stdout);
  fputc('\n', stdout);
 
  
  fprintf(stdout, "%s: Creating new literal string node\n", program);
  node3=librdf_new_node_from_literal(world, lit_string, NULL, 0);
  if(!node3) {
    fprintf(stderr, "%s: librdf_new_node_from_literal failed\n", program);
    return(1);
  }

  buffer=librdf_node_get_literal_value_as_latin1(node3);
  if(!buffer) {
    fprintf(stderr, "%s: Failed to get literal string value as Latin-1\n", program);
    return(1);
  }
  fprintf(stdout, "%s: Node literal string value (Latin-1) is: '%s'\n",
          program, buffer);
  LIBRDF_FREE(cstring, buffer);

  
  fprintf(stdout, "%s: Creating new blank node with identifier %s\n", program, genid);
  node4=librdf_new_node_from_blank_identifier(world, genid);
  if(!node4) {
    fprintf(stderr, "%s: librdf_new_node_from_blank_identifier failed\n", program);
    return(1);
  }

  buffer=librdf_node_get_blank_identifier(node4);
  if(!buffer) {
    fprintf(stderr, "%s: Failed to get blank node identifier\n", program);
    return(1);
  }
  fprintf(stdout, "%s: Node identifier is: '%s'\n", program, buffer);
  
  node5=librdf_new_node_from_node(node4);
  if(!node5) {
    fprintf(stderr, "%s: Failed to make new blank node from old one\n", program);
    return(1);
  } 

  buffer=librdf_node_get_blank_identifier(node5);
  if(!buffer) {
    fprintf(stderr, "%s: Failed to get copied blank node identifier\n", program);
    return(1);
  }
  fprintf(stdout, "%s: Copied node identifier is: '%s'\n", program, buffer);


  uri2=librdf_new_uri(world, "http://example.org/datatypeURI");
  node6=librdf_new_node_from_typed_literal(world, "Datatyped literal value",
                                           "en-GB", uri2);
  librdf_free_uri(uri2);

  size=librdf_node_encode(node6, NULL, 0);
  fprintf(stdout, "%s: Encoding typed node requires %d bytes\n", program, size);
  buffer=(char*)LIBRDF_MALLOC(cstring, size);

  fprintf(stdout, "%s: Encoding typed node in buffer\n", program);
  size2=librdf_node_encode(node6, (unsigned char*)buffer, size);
  if(size2 != size) {
    fprintf(stderr, "%s: Encoding typed node used %d bytes, expected it to use %d\n", program, size2, size);
    return(1);
  }
  
  fprintf(stdout, "%s: Decoding typed node from buffer\n", program);
  if(!(node7=librdf_node_decode(world, NULL, (unsigned char*)buffer, size))) {
    fprintf(stderr, "%s: Decoding typed node failed\n", program);
    return(1);
  }
  LIBRDF_FREE(cstring, buffer);
   
    


  fprintf(stdout, "%s: Freeing nodes\n", program);
  librdf_free_node(node7);
  librdf_free_node(node6);
  librdf_free_node(node5);
  librdf_free_node(node4);
  librdf_free_node(node3);
  librdf_free_node(node2);
  librdf_free_node(node);

  librdf_finish_node(world);
  librdf_finish_uri(world);
  librdf_finish_hash(world);
  librdf_finish_digest(world);

  LIBRDF_FREE(librdf_world, world);

  /* keep gcc -Wall happy */
  return(0);
}

#endif
