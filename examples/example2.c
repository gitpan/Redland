/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * example2.c - Redland example code parsing RDF/XML from a string in memory and adding/checking/removing a statement
 *
 * $Id: example2.c,v 1.16 2003/08/26 21:17:34 cmdjb Exp $
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


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <redland.h>

/* one prototype needed */
int main(int argc, char *argv[]);


static const char *rdfxml_content="<?xml version=\"1.0\"?>\
<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\
     xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\
  <rdf:Description rdf:about=\"http://purl.org/net/dajobe/\">\
    <dc:title>Dave Beckett's Home Page</dc:title>\
    <dc:creator>Dave Beckett</dc:creator>\
    <dc:description>The generic home page of Dave Beckett.</dc:description>\
  </rdf:Description> \
</rdf:RDF>\
";

int
main(int argc, char *argv[]) 
{
  librdf_world* world;
  librdf_storage* storage;
  librdf_model* model;
  librdf_parser* parser;
  librdf_statement* statement;
  librdf_uri* uri;
  char *program=argv[0];
  int rc=0;
  
  world=librdf_new_world();
  librdf_world_open(world);


  uri=librdf_new_uri(world, "http://example.librdf.org/");
  
  if(!uri) {
    fprintf(stderr, "%s: Failed to create URI\n", program);
    return(1);
  }

  storage=librdf_new_storage(world, "memory", "test", NULL);
  if(!storage) {
    fprintf(stderr, "%s: Failed to create new storage\n", program);
    rc=1;
    goto tidyworld;
  }

  model=librdf_new_model(world, storage, NULL);
  if(!model) {
    fprintf(stderr, "%s: Failed to create model\n", program);
    rc=1;
    goto tidystorage;
  }

  parser=librdf_new_parser(world, "raptor", NULL, NULL);
  if(!parser) {
    fprintf(stderr, "%s: Failed to create new parser 'raptor'\n", program);
    rc=1;
    goto tidystorage;
  }


  if(librdf_parser_parse_string_into_model(parser, rdfxml_content, uri, model)) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", program);
    librdf_free_uri(uri);
    rc=1;
    goto tidymodel;
  }

  librdf_free_parser(parser); parser=NULL;
  
  librdf_free_uri(uri); uri=NULL;


  statement=librdf_new_statement(world);
  if(!statement) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", program);
    rc=1;
    goto tidymodel;
  }
  
  librdf_statement_set_subject(statement, 
                               librdf_new_node_from_uri_string(world, "http://example.org/subject"));
  
  librdf_statement_set_predicate(statement,
                                   librdf_new_node_from_uri_string(world, "http://example.org/pred1"));
  
  librdf_statement_set_object(statement,
                              librdf_new_node_from_literal(world, "object", NULL, 0));
  
  librdf_model_add_statement(model, statement);

  fprintf(stdout, "%s: Resulting model is:\n", program);
  librdf_model_print(model, stdout);

  if(!librdf_model_contains_statement(model, statement)) {
    fprintf(stdout, "%s: Model does not contain statement\n", program);
    rc=1;
    goto tidystatement;
  } else
    fprintf(stdout, "%s: Model contains the statement\n", program);


  fprintf(stdout, "%s: Removing the statement\n", program);
  librdf_model_remove_statement(model, statement);

  fprintf(stdout, "%s: Resulting model is:\n", program);
  librdf_model_print(model, stdout);

 tidystatement:
  librdf_free_statement(statement);

 tidymodel:
  librdf_free_model(model);

 tidystorage:
  librdf_free_storage(storage);

 tidyworld:
  librdf_free_world(world);

#ifdef LIBRDF_MEMORY_DEBUG
  librdf_memory_report(stderr);
#endif
	
  /* keep gcc -Wall happy */
  return(rc);
}
