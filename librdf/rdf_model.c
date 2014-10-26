/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_model.c - RDF Model implementation
 *
 * $Id: rdf_model.c,v 1.66 2003/08/31 11:26:16 cmdjb Exp $
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
#include <stdlib.h> /* for exit()  */
#endif

#include <librdf.h>

#ifndef STANDALONE
/* prototypes for helper functions */
static void librdf_delete_model_factories(void);


/**
 * librdf_init_model - Initialise librdf_model class
 * @world: redland world object
 **/
void
librdf_init_model(librdf_world *world)
{
  /* Always have model storage implementation available */
  librdf_init_model_storage();
}


/**
 * librdf_finish_model - Terminate librdf_model class
 * @world: redland world object
 **/
void
librdf_finish_model(librdf_world *world)
{
  librdf_delete_model_factories();
}


/* statics */

/* list of storage factories */
static librdf_model_factory* models;


/*
 * librdf_delete_model_factories - helper function to delete all the registered model factories
 */
static void
librdf_delete_model_factories(void)
{
  librdf_model_factory *factory, *next;
  
  for(factory=models; factory; factory=next) {
    next=factory->next;
    LIBRDF_FREE(librdf_model_factory, factory->name);
    LIBRDF_FREE(librdf_model_factory, factory);
  }
  models=NULL;
}


/* class methods */

/**
 * librdf_model_register_factory - Register a model factory
 * @name: the model factory name
 * @factory: pointer to function to call to register the factory
 * 
 **/
void
librdf_model_register_factory(const char *name,
                              void (*factory) (librdf_model_factory*)) 
{
  librdf_model_factory *model, *h;
  char *name_copy;
  
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG2(librdf_model_register_factory,
                "Received registration for model %s\n", name);
#endif
  
  model=(librdf_model_factory*)LIBRDF_CALLOC(librdf_model_factory, 1,
                                             sizeof(librdf_model_factory));
  if(!model)
    LIBRDF_FATAL1(world, librdf_model_register_factory, "Out of memory");

  name_copy=(char*)LIBRDF_CALLOC(cstring, strlen(name)+1, 1);
  if(!name_copy) {
    LIBRDF_FREE(librdf_model, model);
    LIBRDF_FATAL1(world, librdf_model_register_factory, "Out of memory");
  }
  strcpy(name_copy, name);
  model->name=name_copy;
        
  for(h = models; h; h = h->next ) {
    if(!strcmp(h->name, name_copy)) {
      LIBRDF_FREE(cstring, name_copy);
      LIBRDF_ERROR2(model->world, librdf_model_register_factory,
                    "model %s already registered", h->name);
      return;
    }
  }
  
  /* Call the model registration function on the new object */
  (*factory)(model);
  
#if defined(LIBRDF_DEBUG) && LIBRDF_DEBUG > 1
  LIBRDF_DEBUG3(librdf_model_register_factory, "%s has context size %d\n",
                name, model->context_length);
#endif
  
  model->next = models;
  models = model;
}


/**
 * librdf_get_model_factory - Get a model factory by name
 * @name: the factory name or NULL for the default factory
 * 
 * Return value: the factory object or NULL if there is no such factory
 **/
librdf_model_factory*
librdf_get_model_factory (const char *name) 
{
  librdf_model_factory *factory;

  /* return 1st model if no particular one wanted - why? */
  if(!name) {
    factory=models;
    if(!factory) {
      LIBRDF_DEBUG1(librdf_get_model_factory, 
                    "No (default) models registered\n");
      return NULL;
    }
  } else {
    for(factory=models; factory; factory=factory->next) {
      if(!strcmp(factory->name, name)) {
        break;
      }
    }
    /* else FACTORY name not found */
    if(!factory) {
      LIBRDF_DEBUG2(librdf_get_model_factory, "No model with name %s found\n",
                    name);
      return NULL;
    }
  }
        
  return factory;
}




/**
 * librdf_new_model - Constructor - create a new storage librdf_model object
 * @world: redland world object
 * @storage: &librdf_storage to use
 * @options_string: options to initialise model
 *
 * The options are encoded as described in librdf_hash_from_string()
 * and can be NULL if none are required.
 *
 * Return value: a new &librdf_model object or NULL on failure
 */
librdf_model*
librdf_new_model (librdf_world *world,
                  librdf_storage *storage, char *options_string) {
  librdf_hash* options_hash;
  librdf_model *model;

  if(!storage)
    return NULL;
  
  options_hash=librdf_new_hash(world, NULL);
  if(!options_hash)
    return NULL;
  
  if(librdf_hash_from_string(options_hash, options_string)) {
    librdf_free_hash(options_hash);
    return NULL;
  }

  model=librdf_new_model_with_options(world, storage, options_hash);
  librdf_free_hash(options_hash);
  return model;
}


/**
 * librdf_new_model_with_options - Constructor - Create a new librdf_model with storage
 * @world: redland world object
 * @storage: &librdf_storage storage to use
 * @options: &librdf_hash of options to use
 * 
 * Options are presently not used.
 *
 * Return value: a new &librdf_model object or NULL on failure
 **/
librdf_model*
librdf_new_model_with_options(librdf_world *world,
                              librdf_storage *storage, librdf_hash* options)
{
  librdf_model *model;
  
  if(!storage)
    return NULL;
  
  model=(librdf_model*)LIBRDF_CALLOC(librdf_model, 1, sizeof(librdf_model));
  if(!model)
    return NULL;
  
  model->world=world;

  model->factory=librdf_get_model_factory("storage");
  if(!model->factory) {
    LIBRDF_FREE(librdf_model, model);
    return NULL;
  }
    
  model->context=LIBRDF_MALLOC(data, model->factory->context_length);

  if(model->context && model->factory->create(model, storage, options)) {
    LIBRDF_FREE(data, model->context);
    LIBRDF_FREE(librdf_model, model);
    return NULL;
  }
  
  return model;
}


/**
 * librdf_new_model_from_model - Copy constructor - create a new librdf_model from an existing one
 * @model: the existing &librdf_model
 * 
 * Creates a new model as a copy of the existing model in the same
 * storage context.
 * 
 * Return value: a new &librdf_model or NULL on failure
 **/
librdf_model*
librdf_new_model_from_model(librdf_model* model)
{
  return model->factory->clone(model);
}


/**
 * librdf_free_model - Destructor - Destroy a librdf_model object
 * @model: &librdf_model model to destroy
 * 
 **/
void
librdf_free_model(librdf_model *model)
{
  librdf_iterator* iterator;
  librdf_model* m;

  if(model->sub_models) {
    iterator=librdf_list_get_iterator(model->sub_models);
    if(iterator) {
      while(!librdf_iterator_end(iterator)) {
        m=(librdf_model*)librdf_iterator_get_object(iterator);
        if(m)
          librdf_free_model(m);
        librdf_iterator_next(iterator);
      }
      librdf_free_iterator(iterator);
    }
    librdf_free_list(model->sub_models);
  } else {
    model->factory->destroy(model);
  }
  LIBRDF_FREE(data, model->context);

  LIBRDF_FREE(librdf_model, model);
}



/**
 * librdf_model_size - get the number of statements in the model
 * @model: &librdf_model object
 * 
 * WARNING: Not all underlying stores can return the size of the graph
 * In which case the return value will be negative.
 *
 * Return value: the number of statements or <0 if not possible
 **/
int
librdf_model_size(librdf_model* model)
{
  return model->factory->size(model);
}


/**
 * librdf_model_add_statement - Add a statement to the model
 * @model: model object
 * @statement: statement object
 * 
 * The passed-in statement is copied when added to the model, not
 * shared with the model.  It must be a complete statement - all
 * of subject, predicate, object parts must be present.
 *
 * Return value: non 0 on failure
 **/
int
librdf_model_add_statement(librdf_model* model, librdf_statement* statement)
{
  if(!librdf_statement_is_complete(statement))
    return 1;
  
  return model->factory->add_statement(model, statement);
}


/**
 * librdf_model_add_statements - Add a stream of statements to the model
 * @model: model object
 * @statement_stream: stream of statements to use
 * 
 * Return value: non 0 on failure
 **/
int
librdf_model_add_statements(librdf_model* model, librdf_stream* statement_stream)
{
  return model->factory->add_statements(model, statement_stream);
}


/**
 * librdf_model_add - Create and add a new statement about a resource to the model
 * @model: model object
 * @subject: &librdf_node of subject
 * @predicate: &librdf_node of predicate
 * @object: &librdf_node of object (literal or resource)
 * 
 * After this method, the &librdf_node objects become owned by the model.
 * All of subject, predicate and object must be non-NULL.
 * 
 * Return value: non 0 on failure
 **/
int
librdf_model_add(librdf_model* model, librdf_node* subject, 
		 librdf_node* predicate, librdf_node* object)
{
  librdf_statement *statement;
  int result;
  
  if(!subject || !predicate || !object)
    return 1;

  statement=librdf_new_statement(model->world);
  if(!statement)
    return 1;

  librdf_statement_set_subject(statement, subject);
  librdf_statement_set_predicate(statement, predicate);
  librdf_statement_set_object(statement, object);

  result=librdf_model_add_statement(model, statement);
  librdf_free_statement(statement);
  
  return result;
}


/**
 * librdf_model_add_typed_literal_statement - Create and add a new statement about a typed literal to the model
 * @model: model object
 * @subject: &librdf_node of subject
 * @predicate: &librdf_node of predicate
 * @literal: string literal content
 * @xml_language: language of literal
 * @datatype: datatype librdf_uri
 * 
 * After this method, the &librdf_node subject and predicate become
 * owned by the model.
 * 
 * The language can be set to NULL if not used.
 * All of subject, predicate and literal must be non-NULL.
 *
 * Return value: non 0 on failure
 **/
int
librdf_model_add_typed_literal_statement(librdf_model* model, 
                                         librdf_node* subject, 
                                         librdf_node* predicate, char* literal,
                                         char *xml_language,
                                         librdf_uri *datatype_uri)
{
  librdf_node* object;

  if(!subject || !predicate || !literal)
    return 1;

  object=librdf_new_node_from_typed_literal(model->world,
                                            literal, xml_language, 
                                            datatype_uri);
  if(!object)
    return 1;
  
  return librdf_model_add(model, subject, predicate, object);
}


/**
 * librdf_model_add_string_literal_statement - Create and add a new statement about a literal to the model
 * @model: model object
 * @subject: &librdf_node of subject
 * @predicate: &librdf_node of predicate
 * @literal: string literal conten
 * @xml_language: language of literal
 * @is_wf_xml: literal is XML
 * 
 * The language can be set to NULL if not used.
 * All of subject, predicate and literal must be non-NULL.
 *
 * 0.9.12: xml_space argument deleted
 *
 * Return value: non 0 on failure
 **/
int
librdf_model_add_string_literal_statement(librdf_model* model, 
					  librdf_node* subject, 
					  librdf_node* predicate, char* literal,
					  char *xml_language,
                                          int is_wf_xml)
{
  librdf_node* object;
  int result;
  
  if(!subject || !predicate || !literal)
    return 1;

  object=librdf_new_node_from_literal(model->world,
                                      literal, xml_language, 
                                      is_wf_xml);
  if(!object)
    return 1;
  
  result=librdf_model_add(model, subject, predicate, object);
  if(result)
    librdf_free_node(object);
  
  return result;
}


/**
 * librdf_model_remove_statement - Remove a known statement from the model
 * @model: the model object
 * @statement: the statement
 *
 * It must be a complete statement - all of subject, predicate, object
 * parts must be present.
 *
 * Return value: non 0 on failure
 **/
int
librdf_model_remove_statement(librdf_model* model, librdf_statement* statement)
{
  if(!librdf_statement_is_complete(statement))
    return 1;

  return model->factory->remove_statement(model, statement);
}


/**
 * librdf_model_contains_statement - Check for a statement in the model
 * @model: the model object
 * @statement: the statement
 * 
 * It must be a complete statement - all of subject, predicate, object
 * parts must be present.  Use librdf_model_find_statements to search
 * for partial statement matches.
 *
 * WARNING: librdf_model_contains_statement may not work correctly
 * with stores using contexts.  In this case, a search using
 * librdf_model_find_statements for a non-empty list will
 * return the correct result.
 *
 * Return value: non 0 if the model contains the statement
 **/
int
librdf_model_contains_statement(librdf_model* model, librdf_statement* statement)
{
  if(!librdf_statement_is_complete(statement))
    return 1;

  return model->factory->contains_statement(model, statement);
}


/**
 * librdf_model_as_stream - list the model contents as a stream of statements
 * @model: the model object
 * 
 * Return value: a &librdf_stream or NULL on failure
 **/
librdf_stream*
librdf_model_as_stream(librdf_model* model)
{
  return model->factory->serialise(model);
}


/**
 * librdf_model_serialise - serialise the entire model as a stream (DEPRECATED)
 * @model: the model object
 * 
 * DEPRECATED to reduce confusion with the librdf_serializer class.
 * Please use librdf_model_as_stream.
 *
 * Return value: a &librdf_stream or NULL on failure
 **/
librdf_stream*
librdf_model_serialise(librdf_model* model)
{
  return model->factory->serialise(model);
}


/**
 * librdf_model_find_statements - find matching statements in the model
 * @model: the model object
 * @statement: the partial statement to match
 * 
 * The partial statement is a statement where the subject, predicate
 * and/or object can take the value NULL which indicates a match with
 * any value in the model
 * 
 * Return value: a &librdf_stream of statements (can be empty) or NULL
 * on failure.
 **/
librdf_stream*
librdf_model_find_statements(librdf_model* model, 
                             librdf_statement* statement)
{
  return model->factory->find_statements(model, statement);
}


/**
 * librdf_model_get_sources - return the sources (subjects) of arc in an RDF graph given arc (predicate) and target (object)
 * @model: &librdf_model object
 * @arc: &librdf_node arc
 * @target: &librdf_node target
 * 
 * Searches the model for arcs matching the given arc and target
 * and returns a list of the source &librdf_node objects as an iterator
 * 
 * Return value:  &librdf_iterator of &librdf_node objects (may be empty) or NULL on failure
 **/
librdf_iterator*
librdf_model_get_sources(librdf_model *model,
                         librdf_node *arc, librdf_node *target) 
{
  return model->factory->get_sources(model, arc, target);
}


/**
 * librdf_model_get_arcs - return the arcs (predicates) of an arc in an RDF graph given source (subject) and target (object)
 * @model: &librdf_model object
 * @source: &librdf_node source
 * @target: &librdf_node target
 * 
 * Searches the model for arcs matching the given source and target
 * and returns a list of the arc &librdf_node objects as an iterator
 * 
 * Return value:  &librdf_iterator of &librdf_node objects (may be empty) or NULL on failure
 **/
librdf_iterator*
librdf_model_get_arcs(librdf_model *model,
                      librdf_node *source, librdf_node *target) 
{
  return model->factory->get_arcs(model, source, target);
}


/**
 * librdf_model_get_targets - return the targets (objects) of an arc in an RDF graph given source (subject) and arc (predicate)
 * @model: &librdf_model object
 * @source: &librdf_node source
 * @arc: &librdf_node arc
 * 
 * Searches the model for targets matching the given source and arc
 * and returns a list of the source &librdf_node objects as an iterator
 * 
 * Return value:  &librdf_iterator of &librdf_node objects (may be empty) or NULL on failure
 **/
librdf_iterator*
librdf_model_get_targets(librdf_model *model,
                         librdf_node *source, librdf_node *arc) 
{
  return model->factory->get_targets(model, source, arc);
}


/**
 * librdf_model_get_source - return one source (subject) of arc in an RDF graph given arc (predicate) and target (object)
 * @model: &librdf_model object
 * @arc: &librdf_node arc
 * @target: &librdf_node target
 * 
 * Searches the model for arcs matching the given arc and target
 * and returns one &librdf_node object
 * 
 * Return value:  a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_model_get_source(librdf_model *model,
                        librdf_node *arc, librdf_node *target) 
{
  librdf_iterator *iterator=librdf_model_get_sources(model, arc, target);
  librdf_node *node=(librdf_node*)librdf_iterator_get_object(iterator);
  if(node)
    node=librdf_new_node_from_node(node);
  librdf_free_iterator(iterator);
  return node;
}


/**
 * librdf_model_get_arc - return one arc (predicate) of an arc in an RDF graph given source (subject) and target (object)
 * @model: &librdf_model object
 * @source: &librdf_node source
 * @target: &librdf_node target
 * 
 * Searches the model for arcs matching the given source and target
 * and returns one &librdf_node object
 * 
 * Return value:  a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_model_get_arc(librdf_model *model,
                     librdf_node *source, librdf_node *target) 
{
  librdf_iterator *iterator=librdf_model_get_arcs(model, source, target);
  librdf_node *node=(librdf_node*)librdf_iterator_get_object(iterator);
  if(node)
    node=librdf_new_node_from_node(node);
  librdf_free_iterator(iterator);
  return node;
}


/**
 * librdf_model_get_target - return one target (object) of an arc in an RDF graph given source (subject) and arc (predicate)
 * @model: &librdf_model object
 * @source: &librdf_node source
 * @arc: &librdf_node arc
 * 
 * Searches the model for targets matching the given source and arc
 * and returns one &librdf_node object
 * 
 * Return value:  a new &librdf_node object or NULL on failure
 **/
librdf_node*
librdf_model_get_target(librdf_model *model,
                        librdf_node *source, librdf_node *arc) 
{
  librdf_iterator *iterator=librdf_model_get_targets(model, source, arc);
  librdf_node *node=(librdf_node*)librdf_iterator_get_object(iterator);
  if(node)
    node=librdf_new_node_from_node(node);
  librdf_free_iterator(iterator);
  return node;
}


/**
 * librdf_model_add_submodel - add a sub-model to the model
 * @model: the model object
 * @sub_model: the sub model to add
 * 
 * FIXME: Not tested
 * 
 * Return value: non 0 on failure
 **/
int
librdf_model_add_submodel(librdf_model* model, librdf_model* sub_model)
{
  librdf_list *l=model->sub_models;
  
  if(!l) {
    l=librdf_new_list(model->world);
    if(!l)
      return 1;
    model->sub_models=l;
  }
  
  if(!librdf_list_add(l, sub_model))
    return 1;
  
  return 0;
}



/**
 * librdf_model_remove_submodel - remove a sub-model from the model
 * @model: the model object
 * @sub_model: the sub model to remove
 * 
 * FIXME: Not tested
 * 
 * Return value: non 0 on failure
 **/
int
librdf_model_remove_submodel(librdf_model* model, librdf_model* sub_model)
{
  librdf_list *l=model->sub_models;
  
  if(!l)
    return 1;
  if(!librdf_list_remove(l, sub_model))
    return 1;
  
  return 0;
}



/**
 * librdf_model_get_arcs_in - return the properties pointing to the given resource
 * @model: &librdf_model object
 * @node: &librdf_node resource node
 * 
 * Return value:  &librdf_iterator of &librdf_node objects (may be empty) or NULL on failure
 **/
librdf_iterator*
librdf_model_get_arcs_in(librdf_model *model, librdf_node *node) 
{
  return model->factory->get_arcs_in(model, node);
}


/**
 * librdf_model_get_arcs_out - return the properties pointing from the given resource
 * @model: &librdf_model object
 * @node: &librdf_node resource node
 * 
 * Return value:  &librdf_iterator of &librdf_node objects (may be empty) or NULL on failure
 **/
librdf_iterator*
librdf_model_get_arcs_out(librdf_model *model, librdf_node *node) 
{
  return model->factory->get_arcs_out(model, node);
}


/**
 * librdf_model_has_arc_in - check if a node has a given property pointing to it
 * @model: &librdf_model object
 * @node: &librdf_node resource node
 * @property: &librdf_node property node
 * 
 * Return value: non 0 if arc property does point to the resource node
 **/
int
librdf_model_has_arc_in(librdf_model *model, librdf_node *node, 
                        librdf_node *property) 
{
  return model->factory->has_arc_in(model, node, property);
}


/**
 * librdf_model_has_arc_out - check if a node has a given property pointing from it
 * @model: &librdf_model object
 * @node: &librdf_node resource node
 * @property: &librdf_node property node
 * 
 * Return value: non 0 if arc property does point from the resource node
 **/
int
librdf_model_has_arc_out(librdf_model *model, librdf_node *node,
                         librdf_node *property) 
{
  return model->factory->has_arc_out(model, node, property);
}




/**
 * librdf_model_print - print the model
 * @model: the model object
 * @fh: the FILE stream to print to
 * 
 * This method is for debugging and the format of the output should
 * not be relied on.
 **/
void
librdf_model_print(librdf_model *model, FILE *fh)
{
  librdf_stream* stream;
  
  stream=librdf_model_as_stream(model);
  if(!stream)
    return;
  fputs("[[\n", fh);
  librdf_stream_print(stream, fh);
  fputs("]]\n", fh);
  librdf_free_stream(stream);
}


/**
 * librdf_model_context_add_statement - Add a statement to a model with a context
 * @model: &librdf_model object
 * @context: &librdf_node context
 * @statement: &librdf_statement statement object
 * 
 * It must be a complete statement - all
 * of subject, predicate, object parts must be present.
 *
 * Return value: Non 0 on failure
 **/
int
librdf_model_context_add_statement(librdf_model* model, 
                                   librdf_node* context,
                                   librdf_statement* statement) 
{
  if(!librdf_statement_is_complete(statement))
    return 1;

  return model->factory->context_add_statement(model, context, statement);
}



/**
 * librdf_model_context_add_statements - Add statements to a model with a context
 * @model: &librdf_model object
 * @context: &librdf_node context
 * @stream: &librdf_stream stream object
 * 
 * Return value: Non 0 on failure
 **/
int
librdf_model_context_add_statements(librdf_model* model, 
                                    librdf_node* context,
                                    librdf_stream* stream) 
{
  int status=0;
  
  if(!stream)
    return 1;

  if(model->factory->context_add_statements)
    return model->factory->context_add_statements(model, context, stream);

  while(!librdf_stream_end(stream)) {
    librdf_statement* statement=librdf_stream_get_object(stream);
    if(!statement)
      break;
    status=librdf_model_context_add_statement(model, context, statement);
    if(status)
      break;
    librdf_stream_next(stream);
  }

  return status;
}



/**
 * librdf_model_context_remove_statement - Remove a statement from a model in a context
 * @model: &librdf_model object
 * @context: &librdf_uri context
 * @statement: &librdf_statement statement
 * 
 * It must be a complete statement - all of subject, predicate, object
 * parts must be present.
 *
 * Return value: Non 0 on failure
 **/
int
librdf_model_context_remove_statement(librdf_model* model,
                                      librdf_node* context,
                                      librdf_statement* statement) 
{
  if(!librdf_statement_is_complete(statement))
    return 1;

  return model->factory->context_remove_statement(model, context, statement);
}


/**
 * librdf_model_context_remove_statements - Remove statements from a model with the given context
 * @model: &librdf_model object
 * @context: &librdf_uri context
 * 
 * Return value: Non 0 on failure
 **/
int
librdf_model_context_remove_statements(librdf_model* model,
                                       librdf_node* context) 
{
  librdf_stream *stream;
  if(model->factory->context_remove_statements)
    return model->factory->context_remove_statements(model, context);

  stream=librdf_model_context_as_stream(model, context);
  if(!stream)
    return 1;

  while(!librdf_stream_end(stream)) {
    librdf_statement *statement=librdf_stream_get_object(stream);
    if(!statement)
      break;
    librdf_model_context_remove_statement(model, context, statement);
    librdf_stream_next(stream);
  }
  librdf_free_stream(stream);  
  return 0;
}


/**
 * librdf_model_context_as_stream - list all statements in a model context
 * @model: &librdf_model object
 * @context: &librdf_uri context
 * 
 * Return value: &librdf_stream of statements or NULL on failure
 **/
librdf_stream*
librdf_model_context_as_stream(librdf_model* model, librdf_node* context) 
{
  return model->factory->context_serialize(model, context);
}


/**
 * librdf_model_context_serialize - List all statements in a model context
 * @model: &librdf_model object
 * @context: &librdf_uri context
 * 
 * DEPRECATED to reduce confusion with the librdf_serializer class.
 * Please use librdf_model_context_as_stream.
 *
 * Return value: &librdf_stream of statements or NULL on failure
 **/
librdf_stream*
librdf_model_context_serialize(librdf_model* model, librdf_node* context) 
{
  return model->factory->context_serialize(model, context);
}


/**
 * librdf_model_query - Run a query against the model returning matching statements
 * @model: &librdf_model object
 * @query: &librdf_query object
 * 
 * Run the given query against the model and return a &librdf_stream of
 * matching &librdf_statement objects
 * 
 * Return value: &librdf_stream of matching statements (may be empty) or NULL on failure
 **/
librdf_stream*
librdf_model_query(librdf_model* model, librdf_query* query) 
{
  return model->factory->query(model, query);
}


/**
 * librdf_model_query_string - Run a query string against the model returning matching statements
 * @model: &librdf_model object
 * @name: query language name
 * @uri: query language URI (or NULL)
 * @query_string: string in query language
 * 
 * Run the given query against the model and return a &librdf_stream of
 * matching &librdf_statement objects
 * 
 * Return value: &librdf_stream of matching statements (may be empty) or NULL on failure
 **/
librdf_stream*
librdf_model_query_string(librdf_model* model,
                          const char *name, librdf_uri *uri,
                          const char *query_string)
{
  librdf_query *query;
  librdf_stream *stream;

  query=librdf_new_query(model->world, name, uri, query_string);
  if(!query)
    return NULL;
  
  stream=librdf_model_query(model, query);

  librdf_free_query(query);
  
  return stream;
}


/**
 * librdf_model_sync - Synchronise the model to the storage
 * @model: &librdf_model object
 * 
 **/
void
librdf_model_sync(librdf_model* model) 
{
  if(model->factory->sync)
    model->factory->sync(model);
}

#endif


/* TEST CODE */


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);

#define EX1_CONTENT \
"<?xml version=\"1.0\"?>\n" \
"<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n" \
"         xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n" \
"  <rdf:Description rdf:about=\"http://purl.org/net/dajobe/\">\n" \
"    <dc:title>Dave Beckett's Home Page</dc:title>\n" \
"    <dc:creator>Dave Beckett</dc:creator>\n" \
"    <dc:description>The generic home page of Dave Beckett.</dc:description>\n" \
"  </rdf:Description>\n" \
"</rdf:RDF>"

#define EX2_CONTENT \
"<?xml version=\"1.0\"?>\n" \
"<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n" \
"         xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n" \
"  <rdf:Description rdf:about=\"http://purl.org/net/dajobe/\">\n" \
"    <dc:title>Dave Beckett's Home Page</dc:title>\n" \
"    <dc:creator>Dave Beckett</dc:creator>\n" \
"    <dc:description>I do development-based research on RDF, metadata and web searching.</dc:description>\n" \
"    <dc:rights>Copyright &#169; 2002 Dave Beckett</dc:rights>\n" \
"  </rdf:Description>\n" \
"</rdf:RDF>"


int
main(int argc, char *argv[]) 
{
  librdf_storage* storage;
  librdf_model* model;
  librdf_statement *statement;
  librdf_parser* parser;
  librdf_stream* stream;
  const char *parser_name="raptor";
  #define URI_STRING_COUNT 2
  const char *file_uri_strings[URI_STRING_COUNT]={"http://example.org/test1.rdf", "http://example.org/test2.rdf"};
  const char *file_content[URI_STRING_COUNT]={EX1_CONTENT, EX2_CONTENT};
  librdf_uri* uris[URI_STRING_COUNT];
  librdf_node* nodes[URI_STRING_COUNT];
  int i;
  char *program=argv[0];
  /* initialise dependent modules - all of them! */
  librdf_world *world=librdf_new_world();
  librdf_iterator* iterator;
  librdf_node *n1, *n2;
  int count;
  int expected_count;

  librdf_world_open(world);
  
  fprintf(stderr, "%s: Creating storage\n", program);
  if(1) {
    /* test contexts in memory */
    storage=librdf_new_storage(world, NULL, NULL, "contexts='yes'");
  } else {
    /* test contexts on disk */
    storage=librdf_new_storage(world, "hashes", "test", "hash-type='bdb',dir='.',write='yes',new='yes',contexts='yes'");
  }
  if(!storage) {
    fprintf(stderr, "%s: Failed to create new storage\n", program);
    return(1);
  }
  fprintf(stderr, "%s: Creating model\n", program);
  model=librdf_new_model(world, storage, NULL);
  if(!model) {
    fprintf(stderr, "%s: Failed to create new model\n", program);
    return(1);
  }

  statement=librdf_new_statement(world);
  /* after this, nodes become owned by model */
  librdf_statement_set_subject(statement, librdf_new_node_from_uri_string(world, "http://www.ilrt.bris.ac.uk/people/cmdjb/"));
  librdf_statement_set_predicate(statement, librdf_new_node_from_uri_string(world, "http://purl.org/dc/elements/1.1/creator"));

  if(!librdf_model_add_statement(model, statement)) {
    fprintf(stderr, "%s: librdf_model_add_statement unexpectedly succeeded adding a partial statement\n", program);
    return(1);
  }

  librdf_statement_set_object(statement, librdf_new_node_from_literal(world, "Dave Beckett", NULL, 0));

  librdf_model_add_statement(model, statement);
  librdf_free_statement(statement);

  fprintf(stderr, "%s: Printing model\n", program);
  librdf_model_print(model, stderr);
  
  parser=librdf_new_parser(world, parser_name, NULL, NULL);
  if(!parser) {
    fprintf(stderr, "%s: Failed to create new parser type %s\n", program,
            parser_name);
    exit(1);
  }

  for (i=0; i<URI_STRING_COUNT; i++) {
    uris[i]=librdf_new_uri(world, file_uri_strings[i]);
    nodes[i]=librdf_new_node_from_uri_string(world, file_uri_strings[i]);

    fprintf(stderr, "%s: Adding content from %s into statement context\n", program,
            librdf_uri_as_string(uris[i]));
    if(!(stream=librdf_parser_parse_string_as_stream(parser, 
                                                     file_content[i], uris[i]))) {
      fprintf(stderr, "%s: Failed to parse RDF from %s as stream\n", program,
              librdf_uri_as_string(uris[i]));
      exit(1);
    }
    librdf_model_context_add_statements(model, nodes[i], stream);
    librdf_free_stream(stream);

    fprintf(stderr, "%s: Printing model\n", program);
    librdf_model_print(model, stderr);
  }


  fprintf(stderr, "%s: Freeing Parser\n", program);
  librdf_free_parser(parser);


  /* sync - probably a NOP */
  librdf_model_sync(model);


  /* sources */
  n1=librdf_new_node_from_uri_string(world, "http://purl.org/dc/elements/1.1/creator");
  n2=librdf_new_node_from_literal(world, "Dave Beckett", NULL, 0);

  fprintf(stderr, "%s: Looking for sources of arc=", program);
  librdf_node_print(n1, stderr);
  fputs(" target=", stderr);
  librdf_node_print(n2, stderr);
  fputs("\n", stderr);

  iterator=librdf_model_get_sources(model, n1, n2);
  if(!iterator) {
    fprintf(stderr, "%s: librdf_model_get_sources failed\n", program);
    exit(1);
  }

  expected_count=3;
  for(count=0; !librdf_iterator_end(iterator); librdf_iterator_next(iterator), count++) {
    librdf_node* n=(librdf_node*)librdf_iterator_get_object(iterator);
    fputs("  ", stderr);
    librdf_node_print(n, stderr);
    fputs("\n", stderr);
  }
  librdf_free_iterator(iterator);
  if(count != expected_count) {
    fprintf(stderr, "%s: librdf_model_get_sources returned %d nodes, expected %d\n", program, count, expected_count);
    exit(1);
  }
  librdf_free_node(n1);
  librdf_free_node(n2);
  

  /* targets */
  n1=librdf_new_node_from_uri_string(world, "http://purl.org/net/dajobe/");
  n2=librdf_new_node_from_uri_string(world, "http://purl.org/dc/elements/1.1/description");

  fprintf(stderr, "%s: Looking for targets of source=", program);
  librdf_node_print(n1, stderr);
  fputs(" arc=", stderr);
  librdf_node_print(n2, stderr);
  fputs("\n", stderr);

  iterator=librdf_model_get_targets(model, n1, n2);
  if(!iterator) {
    fprintf(stderr, "%s: librdf_model_get_targets failed\n", program);
    exit(1);
  }

  expected_count=2;
  for(count=0; !librdf_iterator_end(iterator); librdf_iterator_next(iterator), count++) {
    librdf_node* n=(librdf_node*)librdf_iterator_get_object(iterator);
    fputs("  ", stderr);
    librdf_node_print(n, stderr);
    fputs("\n", stderr);
  }
  librdf_free_iterator(iterator);
  if(count != expected_count) {
    fprintf(stderr, "%s: librdf_model_get_targets returned %d nodes, expected %d\n", program, count, expected_count);
    exit(1);
  }
  librdf_free_node(n1);
  librdf_free_node(n2);
  

  /* arcs */
  n1=librdf_new_node_from_uri_string(world, "http://purl.org/net/dajobe/");
  n2=librdf_new_node_from_literal(world, "Dave Beckett", NULL, 0);

  fprintf(stderr, "%s: Looking for arcs of source=", program);
  librdf_node_print(n1, stderr);
  fputs(" target=", stderr);
  librdf_node_print(n2, stderr);
  fputs("\n", stderr);

  iterator=librdf_model_get_arcs(model, n1, n2);
  if(!iterator) {
    fprintf(stderr, "%s: librdf_model_get_arcs failed\n", program);
    exit(1);
  }

  expected_count=2;
  for(count=0; !librdf_iterator_end(iterator); librdf_iterator_next(iterator), count++) {
    librdf_node* n=(librdf_node*)librdf_iterator_get_object(iterator);
    fputs("  ", stderr);
    librdf_node_print(n, stderr);
    fputs("\n", stderr);
  }
  librdf_free_iterator(iterator);
  if(count != expected_count) {
    fprintf(stderr, "%s: librdf_model_get_arcs returned %d nodes, expected %d\n", program, count, expected_count);
    exit(1);
  }
  librdf_free_node(n1);
  librdf_free_node(n2);
  

  for (i=0; i<URI_STRING_COUNT; i++) {
    fprintf(stderr, "%s: Removing statement context %s\n", program, 
            librdf_uri_as_string(uris[i]));
    librdf_model_context_remove_statements(model, nodes[i]);

    fprintf(stderr, "%s: Printing model\n", program);
    librdf_model_print(model, stderr);
  }


  fprintf(stderr, "%s: Freeing URIs and Nodes\n", program);
  for (i=0; i<URI_STRING_COUNT; i++) {
    librdf_free_uri(uris[i]);
    librdf_free_node(nodes[i]);
  }
  
  fprintf(stderr, "%s: Freeing model\n", program);
  librdf_free_model(model);

  fprintf(stderr, "%s: Freeing storage\n", program);
  librdf_free_storage(storage);

  librdf_free_world(world);
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif
