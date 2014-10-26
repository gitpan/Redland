/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_namespace.c - Raptor XML namespace classes
 *
 * $Id: raptor_namespace.c,v 1.22 2003/08/13 22:20:18 cmdjb Exp $
 *
 * Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
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
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#nsc-NSDeclared
 * (section 4) says:
 *
 * --------------------------------------------------------------------
 *   The prefix xml is by definition bound to the namespace name 
 *   http://www.w3.org/XML/1998/namespace
 * --------------------------------------------------------------------
 *
 * Errata NE05
 * http://www.w3.org/XML/xml-names-19990114-errata#NE05
 * changes that to read:
 *
 * --------------------------------------------------------------------
 * The prefix xml is by definition bound to the namespace name
 * http://www.w3.org/XML/1998/namespace. It may, but need not, be
 * declared, and must not be bound to any other namespace name. No
 * other prefix may be bound to this namespace name.
 *
 * The prefix xmlns is used only to declare namespace bindings and is
 * by definition bound to the namespace name
 * http://www.w3.org/2000/xmlns/. It must not be declared. No other
 * prefix may be bound to this namespace name.
 *
 * All other prefixes beginning with the three-letter sequence x, m, l,
 * in any case combination, are reserved. This means that
 *  * users should not use them except as defined by later specifications
 *  * processors must not treat them as fatal errors.
 * --------------------------------------------------------------------
 *
 * Thus should define it in the table of namespaces before we start.
 *
 * We *can* also define others, but let's not.
 *
 */

static const char * const raptor_xml_uri="http://www.w3.org/XML/1998/namespace";
static const char * const raptor_rdf_ms_uri=RAPTOR_RDF_MS_URI;
static const char * const raptor_rdf_schema_uri=RAPTOR_RDF_SCHEMA_URI;

void
raptor_namespaces_init(raptor_namespace_stack *nstack,
                       raptor_uri_handler *uri_handler,
                       void *uri_context,
                       raptor_simple_message_handler error_handler,
                       void *error_data)
{
  nstack->top=NULL;
  nstack->uri_handler=uri_handler;
  nstack->uri_context=uri_context;

  nstack->error_handler=error_handler;
  nstack->error_data=error_data;

  nstack->rdf_ms_uri    = uri_handler->new_uri(uri_context, raptor_rdf_ms_uri);
  nstack->rdf_schema_uri= uri_handler->new_uri(uri_context, raptor_rdf_schema_uri);

  /* defined at level -1 since always 'present' when inside the XML world */
  raptor_namespaces_start_namespace_full(nstack, (const unsigned char*)"xml",
                                         (unsigned char*)raptor_xml_uri, -1);
}


void
raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, 
                                  raptor_namespace *nspace)
{
  if(nstack->top)
    nspace->next=nstack->top;
  nstack->top=nspace;
}


int
raptor_namespaces_start_namespace_full(raptor_namespace_stack *nstack, 
                                       const unsigned char *prefix, 
                                       const unsigned char *ns_uri_string,
                                       int depth)

{
  raptor_namespace *ns;

  ns=raptor_namespace_new(nstack, prefix, ns_uri_string, depth);
  if(!ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, ns);
  return 0;
}


void
raptor_namespaces_free(raptor_namespace_stack *nstack) {
  raptor_namespace *ns=nstack->top;
  while(ns) {
    raptor_namespace* next_ns=ns->next;

    raptor_namespace_free(ns);
    ns=next_ns;
  }
  nstack->top=NULL;

  if(nstack->uri_handler) {
    nstack->uri_handler->free_uri(nstack->uri_context, nstack->rdf_ms_uri);
    nstack->uri_handler->free_uri(nstack->uri_context, nstack->rdf_schema_uri);
  }

  nstack->uri_handler=(raptor_uri_handler*)NULL;
  nstack->uri_context=NULL;
}


void 
raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth)
{
  while(nstack->top &&
        nstack->top->depth == depth) {
    raptor_namespace* ns=nstack->top;
    raptor_namespace* next=ns->next;

    RAPTOR_DEBUG3(raptor_namespaces_end_for_depth,
                  "namespace prefix %s depth %d\n",
                  ns->prefix ? (char*)ns->prefix : "(default)", depth);

    raptor_namespace_free(ns);

    nstack->top=next;
  }

}


raptor_namespace*
raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns && ns->prefix; ns=ns->next)
    ;
  return ns;
}


raptor_namespace*
raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, 
                                 const unsigned char *prefix, int prefix_length)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns ; ns=ns->next)
    if(ns->prefix && prefix_length == ns->prefix_length && 
       !strncmp((char*)prefix, (char*)ns->prefix, prefix_length))
      break;
  return ns;
}


int
raptor_namespaces_namespace_in_scope(raptor_namespace_stack *nstack, 
                                     const raptor_namespace *nspace)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns ; ns=ns->next)
    if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nspace->uri))
      return 1;
  return 0;
}


raptor_namespace*
raptor_namespace_new(raptor_namespace_stack *nstack,
                     const unsigned char *prefix, 
                     const unsigned char *ns_uri_string, int depth)
{
  int prefix_length=0;
  int len;
  raptor_namespace *ns;
  unsigned char *p;

  /* Convert an empty namespace string "" to a NULL pointer */
  if(ns_uri_string && !*ns_uri_string)
    ns_uri_string=NULL;

  RAPTOR_DEBUG4(raptor_namespaces_start_namespace,
                "namespace prefix %s uri %s depth %d\n",
                prefix ? (char*)prefix : "(default)", 
                ns_uri_string ? (char*)ns_uri_string : "(none)",
                depth);

  if(prefix && !ns_uri_string) {
    /* failed to find namespace - now what? */
    if(nstack->error_handler)
      nstack->error_handler((raptor_parser*)nstack->error_data, "The namespace URI for prefix \"%s\" is empty.", prefix);
    return NULL;
  }
  

  len=sizeof(raptor_namespace);
  if(prefix) {
    prefix_length=strlen((char*)prefix);
    len+=prefix_length+1;
  }

  /* Just one malloc for structure + namespace (maybe) + prefix (maybe)*/
  ns=(raptor_namespace*)RAPTOR_CALLOC(raptor_namespace, len, 1);
  if(!ns)
    return NULL;

  p=(unsigned char*)ns+sizeof(raptor_namespace);
  if(ns_uri_string) {
    ns->uri=(*nstack->uri_handler->new_uri)(nstack->uri_context, (const char*)ns_uri_string);
    if(!ns->uri) {
      RAPTOR_FREE(raptor_namespace, ns);
      return NULL;
    }
  }
  if(prefix) {
    ns->prefix=(const unsigned char*)strcpy((char*)p, (char*)prefix);
    ns->prefix_length=prefix_length;

    if(!strcmp((char*)ns->prefix, "xml"))
      ns->is_xml=1;
  }
  ns->depth=depth;

  /* set convienience flags when there is a defined namespace URI */
  if(ns_uri_string) {
    if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nstack->rdf_ms_uri))
      ns->is_rdf_ms=1;
    else if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nstack->rdf_schema_uri))
      ns->is_rdf_schema=1;
  }

  ns->nstack=nstack;

  return ns;
}

/* copy to a new stack with a new depth */
int
raptor_namespace_copy(raptor_namespace_stack *nstack,
                      raptor_namespace *ns,
                      int new_depth)
     
{
  raptor_namespace *new_ns;

  char *uri_string=(*nstack->uri_handler->uri_as_string)(nstack->uri_context, ns->uri);

  new_ns=raptor_namespace_new(nstack, ns->prefix, uri_string, new_depth);
  if(!new_ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, new_ns);
  return 0;
}


void 
raptor_namespace_free(raptor_namespace *ns)
{
  if(ns->uri)
    ns->nstack->uri_handler->free_uri(ns->nstack->uri_context, ns->uri);

  RAPTOR_FREE(raptor_namespace, ns);
}


raptor_uri*
raptor_namespace_get_uri(const raptor_namespace *ns) 
{
  return ns->uri;
}


const unsigned char*
raptor_namespace_get_prefix(const raptor_namespace *ns)
{
  return (const unsigned char*)ns->prefix;
}


raptor_uri*
raptor_namespace_local_name_to_uri(const raptor_namespace *ns,
                                   const unsigned char *local_name)
{
  return ns->nstack->uri_handler->new_uri_from_uri_local_name(ns->nstack->uri_context, ns->uri, (const char*)local_name);
}


unsigned char *
raptor_namespaces_format(const raptor_namespace *ns, size_t *length_p)
{
  size_t uri_length;
  const char *uri_string=raptor_uri_as_counted_string(ns->uri, &uri_length);
  size_t length=8+uri_length+ns->prefix_length; /* 8=length of [[xmlns=""] */
  unsigned char *buffer;

  if(ns->prefix)
    length++; /* for : */
  
  if(length_p)
    *length_p=length;

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, length+1);
  if(!buffer)
    return NULL;
  
  if(!uri_length) {
    if(ns->prefix)
      sprintf(buffer, "xmlns:%s=\"\"", ns->prefix);
    else
      strcpy(buffer, "xmlns=\"\"");
  } else {
    if(ns->prefix)
      sprintf(buffer, "xmlns:%s=\"%s\"", ns->prefix, uri_string);
    else
      sprintf(buffer, "xmlns=\"%s\"", uri_string);
  }

  return buffer;
}


#ifdef RAPTOR_DEBUG
void
raptor_namespace_print(FILE *stream, raptor_namespace* ns) 
{
  const char *uri_string=raptor_uri_as_string(ns->uri);
  if(ns->prefix)
    fprintf(stream, "%s:%s", ns->prefix, uri_string);
  else
    fprintf(stream, "(default):%s", uri_string);
}
#endif



#ifdef STANDALONE


/* one more prototype */
int main(int argc, char *argv[]);


#ifdef RAPTOR_IN_REDLAND
#include <librdf.h>
#endif

int
main(int argc, char *argv[]) 
{
  raptor_namespace_stack namespaces;
  raptor_uri_handler *handler;
  void *context;

#ifdef RAPTOR_IN_REDLAND
  /* Redland initialises the raptor URI class during these calls: */
  librdf_world *world=librdf_new_world();
  librdf_world_open(world);
#else
  raptor_uri_init();
#endif

  /* Use whatever the raptor_uri class has */
  raptor_uri_get_handler(&handler, &context);

  raptor_namespaces_init(&namespaces, handler, context, NULL, NULL);
  
  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex1",
                                         (const unsigned char*)"http://example.org/ns1",
                                         0);

  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex2",
                                         (const unsigned char*)"http://example.org/ns2",
                                         1);

  raptor_namespaces_end_for_depth(&namespaces, 1);

  raptor_namespaces_end_for_depth(&namespaces, 0);

  raptor_namespaces_free(&namespaces);

#ifdef RAPTOR_IN_REDLAND
  librdf_free_world(world);
#endif
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif

/*
 * Local Variables:
 * mode:c
 * c-basic-offset: 2
 * End:
 */
