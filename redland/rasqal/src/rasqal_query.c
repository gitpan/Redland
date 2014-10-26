/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal_query.c - Rasqal RDF Query
 *
 * $Id: rasqal_query.c 11496 2006-10-14 23:07:01Z dajobe $
 *
 * Copyright (C) 2003-2006, David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include <rasqal_config.h>
#endif

#ifdef WIN32
#include <win32_rasqal_config.h>
#endif

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>

#include "rasqal.h"
#include "rasqal_internal.h"


static void rasqal_query_add_query_result(rasqal_query* query, rasqal_query_results* query_results);
static int rasqal_query_write_sparql_20060406(raptor_iostream *iostr, rasqal_query* query, raptor_uri *base_uri);



/**
 * rasqal_new_query:
 * @name: the query language name (or NULL)
 * @uri: #raptor_uri language uri (or NULL)
 *
 * Constructor - create a new rasqal_query object.
 *
 * A query language can be named or identified by a URI, either
 * of which is optional.  The default query language will be used
 * if both are NULL.  rasqal_languages_enumerate returns
 * information on the known names, labels and URIs.
 *
 * Return value: a new #rasqal_query object or NULL on failure
 */
rasqal_query*
rasqal_new_query(const char *name, const unsigned char *uri)
{
  rasqal_query_engine_factory* factory;
  rasqal_query* query;
  raptor_uri_handler *uri_handler;
  void *uri_context;

  factory=rasqal_get_query_engine_factory(name, uri);
  if(!factory)
    return NULL;

  query=(rasqal_query*)RASQAL_CALLOC(rasqal_query, 1, sizeof(rasqal_query));
  if(!query)
    return NULL;
  
  query->context=(char*)RASQAL_CALLOC(rasqal_query_context, 1,
                                      factory->context_length);
  if(!query->context) {
    rasqal_free_query(query);
    return NULL;
  }
  
  query->factory=factory;

  query->failed=0;

  raptor_uri_get_handler(&uri_handler, &uri_context);
  query->namespaces=raptor_new_namespaces(uri_handler, uri_context,
                                          (raptor_simple_message_handler)rasqal_query_simple_error,
                                          query,
                                          0);

  query->variables_sequence=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_variable, (raptor_sequence_print_handler*)rasqal_variable_print);

  query->anon_variables_sequence=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_variable, (raptor_sequence_print_handler*)rasqal_variable_print);

  query->triples=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_triple, (raptor_sequence_print_handler*)rasqal_triple_print);
  
  query->prefixes=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_prefix, (raptor_sequence_print_handler*)rasqal_prefix_print);

  query->query_graph_pattern=NULL;

  query->data_graphs=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_data_graph, (raptor_sequence_print_handler*)rasqal_data_graph_print);

  query->distinct= 0;
  query->limit= -1;
  query->offset= -1;

  query->order_conditions_sequence=NULL;

  query->usage=1;
  
  if(factory->init(query, name)) {
    rasqal_free_query(query);
    return NULL;
  }
  
  return query;
}



/**
 * rasqal_free_query:
 * @query: #rasqal_query object
 * 
 * Destructor - destroy a #rasqal_query object.
 **/
void
rasqal_free_query(rasqal_query* query) 
{
  if(--query->usage)
    return;
  
  if(query->factory)
    query->factory->terminate(query);

  if(query->context)
    RASQAL_FREE(rasqal_query_context, query->context);

  if(query->namespaces)
    raptor_free_namespaces(query->namespaces);

  if(query->base_uri)
    raptor_free_uri(query->base_uri);

  if(query->query_string)
    RASQAL_FREE(cstring, query->query_string);

  if(query->data_graphs)
    raptor_free_sequence(query->data_graphs);
  if(query->selects)
    raptor_free_sequence(query->selects);
  if(query->describes)
    raptor_free_sequence(query->describes);

  if(query->triples)
    raptor_free_sequence(query->triples);
  if(query->optional_triples)
    raptor_free_sequence(query->optional_triples);
  if(query->constructs)
    raptor_free_sequence(query->constructs);
  if(query->prefixes)
    raptor_free_sequence(query->prefixes);

  if(query->variable_names)
    RASQAL_FREE(cstrings, query->variable_names);
  
  if(query->binding_values)
    RASQAL_FREE(cstrings, query->binding_values);
  
  if(query->variables)
    RASQAL_FREE(vararray, query->variables);

  if(query->variables_declared_in)
    RASQAL_FREE(intarray, query->variables_declared_in);

  if(query->query_graph_pattern)
    rasqal_free_graph_pattern(query->query_graph_pattern);

  if(query->order_conditions_sequence)
    raptor_free_sequence(query->order_conditions_sequence);

  /* Do this last since most everything above could refer to a variable */
  if(query->anon_variables_sequence)
    raptor_free_sequence(query->anon_variables_sequence);

  if(query->variables_sequence)
    raptor_free_sequence(query->variables_sequence);

  if(query->graph_patterns_sequence)
    raptor_free_sequence(query->graph_patterns_sequence);

  RASQAL_FREE(rasqal_query, query);
}


/* Methods */

/**
 * rasqal_query_get_name:
 * @query: #rasqal_query query object
 *
 * Get a short name for the query language.
 *
 * Return value: shared string label value
 **/
const char*
rasqal_query_get_name(rasqal_query* query)
{
  return query->factory->name;
}


/**
 * rasqal_query_get_label:
 * @query: #rasqal_query query object
 *
 * Get a readable label for the query language.
 *
 * Return value: shared string label value
 **/
const char*
rasqal_query_get_label(rasqal_query* query)
{
  return query->factory->label;
}


/**
 * rasqal_query_set_fatal_error_handler:
 * @query: the query
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 *
 * Set the query error handling function.
 * 
 * The function will receive callbacks when the query fails.
 * 
 **/
void
rasqal_query_set_fatal_error_handler(rasqal_query* query, void *user_data,
                                     raptor_message_handler handler)
{
  query->fatal_error_user_data=user_data;
  query->fatal_error_handler=handler;
}


/**
 * rasqal_query_set_error_handler:
 * @query: the query
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 *
 * Set the query error handling function.
 * 
 * The function will receive callbacks when the query fails.
 * 
 **/
void
rasqal_query_set_error_handler(rasqal_query* query, void *user_data,
                               raptor_message_handler handler)
{
  query->error_user_data=user_data;
  query->error_handler=handler;
}


/**
 * rasqal_query_set_warning_handler:
 * @query: the query
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 *
 * Set the query warning handling function.
 * 
 * The function will receive callbacks when the query gives a warning.
 * 
 **/
void
rasqal_query_set_warning_handler(rasqal_query* query, void *user_data,
                                 raptor_message_handler handler)
{
  query->warning_user_data=user_data;
  query->warning_handler=handler;
}


/**
 * rasqal_query_set_feature:
 * @query: #rasqal_query query object
 * @feature: feature to set from enumerated #rasqal_feature values
 * @value: integer feature value
 *
 * Set various query features.
 * 
 * The allowed features are available via rasqal_features_enumerate().
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
rasqal_query_set_feature(rasqal_query* query, rasqal_feature feature, int value)
{
  switch(feature) {
    case RASQAL_FEATURE_NO_NET:
      query->features[(int)feature]=value;
      break;
      
    default:
      break;
  }

  return 0;
}


/**
 * rasqal_query_set_feature_string:
 * @query: #rasqal_query query object
 * @feature: feature to set from enumerated #rasqal_feature values
 * @value: feature value
 *
 * Set query features with string values.
 * 
 * The allowed features are available via rasqal_features_enumerate().
 * If the feature type is integer, the value is interpreted as an integer.
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
rasqal_query_set_feature_string(rasqal_query *query, 
                                rasqal_feature feature, 
                                const unsigned char *value)
{
  int value_is_string=(rasqal_feature_value_type(feature) == 1);
  if(!value_is_string)
    return rasqal_query_set_feature(query, feature, atoi((const char*)value));

  return -1;
}


/**
 * rasqal_query_get_feature:
 * @query: #rasqal_query query object
 * @feature: feature to get value
 *
 * Get various query features.
 * 
 * The allowed features are available via rasqal_features_enumerate().
 *
 * Note: no feature value is negative
 *
 * Return value: feature value or < 0 for an illegal feature
 **/
int
rasqal_query_get_feature(rasqal_query *query, rasqal_feature feature)
{
  int result= -1;
  
  switch(feature) {
    case RASQAL_FEATURE_NO_NET:
      result=(query->features[(int)feature] != 0);
      break;

    default:
      break;
  }
  
  return result;
}


/**
 * rasqal_query_get_feature_string:
 * @query: #rasqal_query query object
 * @feature: feature to get value
 *
 * Get query features with string values.
 * 
 * The allowed features are available via rasqal_features_enumerate().
 * If a string is returned, it must be freed by the caller.
 *
 * Return value: feature value or NULL for an illegal feature or no value
 **/
const unsigned char *
rasqal_query_get_feature_string(rasqal_query *query, 
                                rasqal_feature feature)
{
  int value_is_string=(rasqal_feature_value_type(feature) == 1);
  if(!value_is_string)
    return NULL;
  
  return NULL;
}


/**
 * rasqal_query_get_distinct:
 * @query: #rasqal_query query object
 *
 * Get the query distinct results flag.
 *
 * Return value: non-0 if the results should be distinct
 **/
int
rasqal_query_get_distinct(rasqal_query* query)
{
  return query->distinct;
}


/**
 * rasqal_query_set_distinct:
 * @query: #rasqal_query query object
 * @is_distinct: non-0 if distinct
 *
 * Set the query distinct results flag.
 *
 **/
void
rasqal_query_set_distinct(rasqal_query* query, int is_distinct)
{
  query->distinct= (is_distinct != 0) ? 1 : 0;
}


/**
 * rasqal_query_get_limit:
 * @query: #rasqal_query query object
 *
 * Get the query-specified limit on results.
 *
 * This is the limit given in the query on the number of results allowed.
 *
 * Return value: integer >=0 if a limit is given, otherwise <0
 **/
int
rasqal_query_get_limit(rasqal_query* query)
{
  return query->limit;
}


/**
 * rasqal_query_set_limit:
 * @query: #rasqal_query query object
 * @limit: the limit on results, >=0 to set a limit, <0 to have no limit
 *
 * Set the query-specified limit on results.
 *
 * This is the limit given in the query on the number of results allowed.
 **/
void
rasqal_query_set_limit(rasqal_query* query, int limit)
{
  query->limit=limit;
}


/**
 * rasqal_query_get_offset:
 * @query: #rasqal_query query object
 *
 * Get the query-specified offset on results.
 *
 * This is the offset given in the query on the number of results allowed.
 *
 * Return value: integer >=0 if a offset is given, otherwise <0
 **/
int
rasqal_query_get_offset(rasqal_query* query)
{
  return query->offset;
}


/**
 * rasqal_query_set_offset:
 * @query: #rasqal_query query object
 * @offset: offset for results, >=0 to set an offset, <0 to have no offset
 *
 * Set the query-specified offset on results.
 *
 * This is the offset given in the query on the number of results allowed.
 **/
void
rasqal_query_set_offset(rasqal_query* query, int offset)
{
  query->offset=offset;
}


/**
 * rasqal_query_add_data_graph:
 * @query: #rasqal_query query object
 * @uri: #raptor_uri source uri for retrieval
 * @name_uri: #raptor_uri name uri (or NULL)
 * @flags: RASQAL_DATA_GRAPH_NAMED or RASQAL_DATA_GRAPH_BACKGROUND
 *
 * Add a data graph to the query.
 *
 * named_uri must be given if flags RASQAL_DATA_GRAPH_NAMED is set.
 * It is the name of the graph and also used as the base URI
 * when resolving any relative URIs for the graph in uri.
 *
 * Return value: non-0 on failure
 **/
int
rasqal_query_add_data_graph(rasqal_query* query, 
                            raptor_uri* uri, raptor_uri* name_uri,
                            int flags)
{
  rasqal_data_graph *dg;

  if((flags & RASQAL_DATA_GRAPH_NAMED) && !name_uri)
    return 1;
  
  dg=rasqal_new_data_graph(uri, name_uri, flags);
  
  raptor_sequence_push(query->data_graphs, (void*)dg);

  return 0;
}


/**
 * rasqal_query_get_data_graph_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of data_graph URIs.
 *
 * Return value: a #raptor_sequence of #raptor_uri pointers.
 **/
raptor_sequence*
rasqal_query_get_data_graph_sequence(rasqal_query* query)
{
  return query->data_graphs;
}


/**
 * rasqal_query_get_data_graph:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a rasqal_data_graph* in the sequence of data_graphs.
 *
 * Return value: a #rasqal_data_graph pointer or NULL if out of the sequence range
 **/
rasqal_data_graph*
rasqal_query_get_data_graph(rasqal_query* query, int idx)
{
  if(!query->data_graphs)
    return NULL;
  
  return (rasqal_data_graph*)raptor_sequence_get_at(query->data_graphs, idx);
}


/**
 * rasqal_query_add_variable:
 * @query: #rasqal_query query object
 * @var: #rasqal_variable variable
 *
 * Add a binding variable to the query.
 *
 * See also rasqal_query_set_variable which assigns or removes a value to
 * a previously added variable in the query.
 **/
void
rasqal_query_add_variable(rasqal_query* query, rasqal_variable* var)
{
  if(!query->selects)
    query->selects=raptor_new_sequence(NULL, (raptor_sequence_print_handler*)rasqal_variable_print);

  raptor_sequence_push(query->selects, (void*)var);
}


/**
 * rasqal_query_get_bound_variable_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of variables to bind in the query.
 *
 * This returns the sequence of variables that are explicitly chosen
 * via SELECT in RDQL, SPARQL.  Or all variables mentioned with SELECT *
 *
 * Return value: a #raptor_sequence of #rasqal_variable pointers.
 **/
raptor_sequence*
rasqal_query_get_bound_variable_sequence(rasqal_query* query)
{
  return query->selects;
}


/**
 * rasqal_query_get_anonymous_variable_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of anonymous variables mentioned in the query.
 *
 * Return value: a #raptor_sequence of #rasqal_variable pointers.
 **/
raptor_sequence*
rasqal_query_get_anonymous_variable_sequence(rasqal_query* query)
{
  return query->anon_variables_sequence;
}


/**
 * rasqal_query_get_all_variable_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of all variables mentioned in the query.
 *
 * Return value: a #raptor_sequence of #rasqal_variable pointers.
 **/
raptor_sequence*
rasqal_query_get_all_variable_sequence(rasqal_query* query)
{
  return query->variables_sequence;
}


/**
 * rasqal_query_get_variable:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a variable in the sequence of variables to bind.
 *
 * Return value: a #rasqal_variable pointer or NULL if out of the sequence range
 **/
rasqal_variable*
rasqal_query_get_variable(rasqal_query* query, int idx)
{
  if(!query->selects)
    return NULL;
  
  return (rasqal_variable*)raptor_sequence_get_at(query->selects, idx);
}


/**
 * rasqal_query_has_variable:
 * @query: #rasqal_query query object
 * @name: variable name
 *
 * Find if the named variable is in the sequence of variables to bind.
 *
 * Return value: non-0 if the variable name was found.
 **/
int
rasqal_query_has_variable(rasqal_query* query, const unsigned char *name)
{
  int i;

  if(!query->selects)
    return 1;
  
  for(i=0; i< raptor_sequence_size(query->selects); i++) {
    rasqal_variable* v=(rasqal_variable*)raptor_sequence_get_at(query->selects, i);
    if(!strcmp((const char*)v->name, (const char*)name))
      return 1;
  }
  return 0;
}


/**
 * rasqal_query_set_variable:
 * @query: #rasqal_query query object
 * @name: #rasqal_variable variable
 * @value: #rasqal_literal value to set or NULL
 *
 * Add a binding variable to the query.
 *
 * See also rasqal_query_add_variable which adds a new binding variable
 * and must be called before this method is invoked.
 *
 * Return value: non-0 on failure
 **/
int
rasqal_query_set_variable(rasqal_query* query, const unsigned char *name,
                          rasqal_literal* value)
{
  int i;

  if(!query->selects)
    return 1;
  
  for(i=0; i< raptor_sequence_size(query->selects); i++) {
    rasqal_variable* v=(rasqal_variable*)raptor_sequence_get_at(query->selects, i);
    if(!strcmp((const char*)v->name, (const char*)name)) {
      if(v->value)
        rasqal_free_literal(v->value);
      v->value=value;
      return 0;
    }
  }
  return 1;
}


/**
 * rasqal_query_get_triple_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of matching triples in the query.
 *
 * Return value: a #raptor_sequence of #rasqal_triple pointers.
 **/
raptor_sequence*
rasqal_query_get_triple_sequence(rasqal_query* query)
{
  return query->triples;
}


/**
 * rasqal_query_get_triple:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a triple in the sequence of matching triples in the query.
 *
 * Return value: a #rasqal_triple pointer or NULL if out of the sequence range
 **/
rasqal_triple*
rasqal_query_get_triple(rasqal_query* query, int idx)
{
  if(!query->triples)
    return NULL;
  
  return (rasqal_triple*)raptor_sequence_get_at(query->triples, idx);
}


int
rasqal_query_declare_prefix(rasqal_query *rq, rasqal_prefix *p)
{
  if(p->declared)
    return 0;
  
  if(raptor_namespaces_start_namespace_full(rq->namespaces, 
                                            p->prefix, 
                                            raptor_uri_as_string(p->uri),
                                            rq->prefix_depth))
    return 1;
  p->declared=1;
  rq->prefix_depth++;
  return 0;
}


static int
rasqal_query_undeclare_prefix(rasqal_query *rq, rasqal_prefix *prefix)
{
  if(!prefix->declared) {
    prefix->declared=1;
    return 0;
  }
  
  raptor_namespaces_end_for_depth(rq->namespaces, prefix->depth);
  return 0;
}


int
rasqal_query_declare_prefixes(rasqal_query *rq) 
{
  int i;
  
  if(!rq->prefixes)
    return 0;
  
  for(i=0; i< raptor_sequence_size(rq->prefixes); i++) {
    rasqal_prefix* p=(rasqal_prefix*)raptor_sequence_get_at(rq->prefixes, i);
    if(rasqal_query_declare_prefix(rq, p))
      return 1;
  }

  return 0;
}


/**
 * rasqal_query_add_prefix:
 * @query: #rasqal_query query object
 * @prefix: #rasqal_prefix namespace prefix, URI
 *
 * Add a namespace prefix to the query.
 *
 * If the prefix has already been used, the old URI will be overridden.
 **/
void
rasqal_query_add_prefix(rasqal_query* query, rasqal_prefix* prefix)
{
  if(!query->prefixes)
    query->prefixes=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_prefix, (raptor_sequence_print_handler*)rasqal_prefix_print);
  else {
    int i;
    for(i=0; i< raptor_sequence_size(query->prefixes); i++) {
      rasqal_prefix* p=(rasqal_prefix*)raptor_sequence_get_at(query->prefixes, i);
      if(strcmp((const char*)p->prefix, (const char*)prefix->prefix)) {
        rasqal_query_undeclare_prefix(query, p);
        break;
      }
    }
  }

  raptor_sequence_push(query->prefixes, (void*)prefix);
}


/**
 * rasqal_query_get_prefix_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of namespace prefixes in the query.
 *
 * Return value: a #raptor_sequence of #rasqal_prefix pointers.
 **/
raptor_sequence*
rasqal_query_get_prefix_sequence(rasqal_query* query)
{
  return query->prefixes;
}


/**
 * rasqal_query_get_prefix:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a prefix in the sequence of namespsace prefixes in the query.
 *
 * Return value: a #rasqal_prefix pointer or NULL if out of the sequence range
 **/
rasqal_prefix*
rasqal_query_get_prefix(rasqal_query* query, int idx)
{
  if(!query->prefixes)
    return NULL;

  return (rasqal_prefix*)raptor_sequence_get_at(query->prefixes, idx);
}


/**
 * rasqal_query_get_query_graph_pattern:
 * @query: #rasqal_query query object
 *
 * Get the top query graph pattern.
 *
 * Return value: a #rasqal_graph_pattern of the top query graph pattern
 **/
rasqal_graph_pattern*
rasqal_query_get_query_graph_pattern(rasqal_query* query)
{
  return query->query_graph_pattern;
}


/**
 * rasqal_query_get_graph_pattern_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of graph_patterns expressions inside the top query graph pattern.
 *
 * Return value: a #raptor_sequence of #rasqal_graph_pattern pointers.
 **/
raptor_sequence*
rasqal_query_get_graph_pattern_sequence(rasqal_query* query)
{
  return rasqal_graph_pattern_get_sub_graph_pattern_sequence(query->query_graph_pattern);
}


/**
 * rasqal_query_get_graph_pattern:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a graph_pattern in the sequence of graph_pattern expressions in the top query graph pattern.
 *
 * Return value: a #rasqal_graph_pattern pointer or NULL if out of the sequence range
 **/
rasqal_graph_pattern*
rasqal_query_get_graph_pattern(rasqal_query* query, int idx)
{
  return rasqal_graph_pattern_get_sub_graph_pattern(query->query_graph_pattern, idx);
}


/**
 * rasqal_query_get_construct_triples_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of triples for a construct.
 *
 * Return value: a #raptor_sequence of #rasqal_triple pointers.
 **/
raptor_sequence*
rasqal_query_get_construct_triples_sequence(rasqal_query* query)
{
  return query->constructs;
}


/**
 * rasqal_query_get_construct_triple:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a triple in the sequence of construct triples.
 *
 * Return value: a #rasqal_triple pointer or NULL if out of the sequence range
 **/
rasqal_triple*
rasqal_query_get_construct_triple(rasqal_query* query, int idx)
{
  if(!query->constructs)
    return NULL;

  return (rasqal_triple*)raptor_sequence_get_at(query->constructs, idx);
}


/**
 * rasqal_graph_pattern_add_sub_graph_pattern:
 * @graph_pattern: graph pattern to add to
 * @sub_graph_pattern: graph pattern to add inside
 *
 * Add a sub graph pattern to a graph pattern .
 *
 **/
void
rasqal_graph_pattern_add_sub_graph_pattern(rasqal_graph_pattern* graph_pattern,
                                           rasqal_graph_pattern* sub_graph_pattern)
{
  if(!graph_pattern->graph_patterns)
    graph_pattern->graph_patterns=raptor_new_sequence((raptor_sequence_free_handler*)rasqal_free_graph_pattern, (raptor_sequence_print_handler*)rasqal_graph_pattern_print);
  raptor_sequence_push(graph_pattern->graph_patterns, sub_graph_pattern);
}


/**
 * rasqal_graph_pattern_get_triple:
 * @graph_pattern: #rasqal_graph_pattern graph pattern object
 * @idx: index into the sequence of triples in the graph pattern
 *
 * Get a triple inside a graph pattern.
 * 
 * Return value: #rasqal_triple or NULL if out of range
 **/
rasqal_triple*
rasqal_graph_pattern_get_triple(rasqal_graph_pattern* graph_pattern, int idx)
{
  if(!graph_pattern->triples)
    return NULL;

  idx += graph_pattern->start_column;

  if(idx > graph_pattern->end_column)
    return NULL;
  
  return (rasqal_triple*)raptor_sequence_get_at(graph_pattern->triples, idx);
}


/**
 * rasqal_graph_pattern_get_sub_graph_pattern_sequence:
 * @graph_pattern: #rasqal_graph_pattern graph pattern object
 *
 * Get the sequence of graph patterns inside a graph pattern .
 * 
 * Return value:  a #raptor_sequence of #rasqal_graph_pattern pointers.
 **/
raptor_sequence*
rasqal_graph_pattern_get_sub_graph_pattern_sequence(rasqal_graph_pattern* graph_pattern)
{
  return graph_pattern->graph_patterns;
}


/**
 * rasqal_graph_pattern_get_sub_graph_pattern:
 * @graph_pattern: #rasqal_graph_pattern graph pattern object
 * @idx: index into the sequence of sub graph_patterns in the graph pattern
 *
 * Get a sub-graph pattern inside a graph pattern.
 * 
 * Return value: #rasqal_graph_pattern or NULL if out of range
 **/
rasqal_graph_pattern*
rasqal_graph_pattern_get_sub_graph_pattern(rasqal_graph_pattern* graph_pattern, int idx)
{
  if(!graph_pattern->graph_patterns)
    return NULL;

  return (rasqal_graph_pattern*)raptor_sequence_get_at(graph_pattern->graph_patterns, idx);
}


/**
 * rasqal_graph_pattern_get_flags:
 * @graph_pattern: #rasqal_graph_pattern graph pattern object
 *
 * Get the graph pattern flags .
 * 
 * @deprecated: Always returns 0
 *
 * Return value: 0
 **/
int
rasqal_graph_pattern_get_flags(rasqal_graph_pattern* graph_pattern)
{
  RASQAL_DEPRECATED_MESSAGE("use rasqal_graph_pattern_get_operator");
  return 0;
}


/**
 * rasqal_graph_pattern_set_origin:
 * @graph_pattern: #rasqal_graph_pattern graph pattern object
 * @origin: #rasqal_literal variable or URI
 *
 * Get the graph pattern triple origin.
 * 
 * All triples in this graph pattern or contained graph patterns are set
 * to have the given origin.
 **/
void
rasqal_graph_pattern_set_origin(rasqal_graph_pattern* graph_pattern,
                                rasqal_literal *origin)
{
  raptor_sequence* s;
  
  s=graph_pattern->triples;
  if(s) {
    int i;

    /* Flag all the triples in this graph pattern with origin */
    for(i= graph_pattern->start_column; i <= graph_pattern->end_column; i++) {
      rasqal_triple *t=(rasqal_triple*)raptor_sequence_get_at(s, i);
      rasqal_triple_set_origin(t, rasqal_new_literal_from_literal(origin));
    }
  }

  s=graph_pattern->graph_patterns;
  if(s) {
    int i;

    /* Flag all the triples in sub-graph patterns with origin */
    for(i=0; i < raptor_sequence_size(s); i++) {
      rasqal_graph_pattern *gp=(rasqal_graph_pattern*)raptor_sequence_get_at(s, i);
      rasqal_graph_pattern_set_origin(gp, origin);
    }
  }

}


static int
rasqal_query_prepare_count_graph_patterns(rasqal_query* query,
                                          rasqal_graph_pattern* gp,
                                          void* data)
{
  raptor_sequence* seq=(raptor_sequence*)data;

  gp->gp_index=(query->graph_pattern_count++);
  raptor_sequence_push(seq, gp);
  
  return 0;
}


/**
 * rasqal_query_prepare:
 * @query: the #rasqal_query object
 * @query_string: the query string (or NULL)
 * @base_uri: base URI of query string (optional)
 *
 * Prepare a query - typically parse it.
 * 
 * Some query languages may require a base URI to resolve any
 * relative URIs in the query string.  If this is not given,
 * the current directory in the filesystem is used as the base URI.
 *
 * The query string may be NULL in which case it is not parsed
 * and the query parts may be created by API calls such as
 * rasqal_query_add_source etc.
 *
 * Return value: non-0 on failure.
 **/
int
rasqal_query_prepare(rasqal_query* query,
                     const unsigned char *query_string,
                     raptor_uri *base_uri)
{
  int rc=0;
  
  if(query->failed)
    return 1;

  if(query->prepared)
    return 0;
  query->prepared=1;

  if(query_string) {
    query->query_string=(unsigned char*)RASQAL_MALLOC(cstring, strlen((const char*)query_string)+1);
    strcpy((char*)query->query_string, (const char*)query_string);
  }

  if(base_uri)
    base_uri=raptor_uri_copy(base_uri);
  else {
    unsigned char *uri_string=raptor_uri_filename_to_uri_string("");
    base_uri=raptor_new_uri(uri_string);
    raptor_free_memory(uri_string);
  }
  
  query->base_uri=base_uri;
  query->locator.uri=base_uri;
  query->locator.line = query->locator.column = query->locator.byte = -1;

  rc=query->factory->prepare(query);
  if(rc)
    query->failed=1;

  if(query->query_graph_pattern) {
    int modified;
    
#if RASQAL_DEBUG > 1
    fputs("Initial query graph pattern:\n  ", stdout);
    rasqal_graph_pattern_print(query->query_graph_pattern, stdout);
    fputs("\n", stdout);
#endif

    do {
      modified=0;
      
      rasqal_query_graph_pattern_visit(query, 
                                       rasqal_engine_merge_triples,
                                       &modified);
      
#if RASQAL_DEBUG > 1
      fprintf(stdout, "modified=%d after merge triples, query graph pattern now:\n  ", modified);
      rasqal_graph_pattern_print(query->query_graph_pattern, stdout);
      fputs("\n", stdout);
#endif

      rasqal_query_graph_pattern_visit(query,
                                       rasqal_engine_remove_empty_group_graph_patterns,
                                       &modified);
      
#if RASQAL_DEBUG > 1
      fprintf(stdout, "modified=%d after remove empty groups, query graph pattern now:\n  ", modified);
      rasqal_graph_pattern_print(query->query_graph_pattern, stdout);
      fputs("\n", stdout);
#endif

      rasqal_query_graph_pattern_visit(query, 
                                       rasqal_engine_merge_graph_patterns,
                                       &modified);

#if RASQAL_DEBUG > 1
      fprintf(stdout, "modified=%d  after merge graph patterns, query graph pattern now:\n  ", modified);
      rasqal_graph_pattern_print(query->query_graph_pattern, stdout);
      fputs("\n", stdout);
#endif

    } while(modified);

    /* Label all graph patterns with an index 0.. for use in discovering
     * the size of the graph pattern execution data array
     */
    query->graph_pattern_count=0;

    /* This sequence stoees shared pointers to the grpah patterns it
     * finds, indexed by the gp_index
     */
    query->graph_patterns_sequence=raptor_new_sequence(NULL, NULL);

    rasqal_query_graph_pattern_visit(query, 
                                     rasqal_query_prepare_count_graph_patterns,
                                     query->graph_patterns_sequence);
    
    rasqal_engine_build_constraints_expression(query->query_graph_pattern);
  }

  return rc;
}



/**
 * rasqal_query_execute:
 * @query: the #rasqal_query object
 *
 * Excute a query - run and return results.
 *
 * return value: a #rasqal_query_results structure or NULL on failure.
 **/
rasqal_query_results*
rasqal_query_execute(rasqal_query* query)
{
  rasqal_query_results *query_results=NULL;
  int rc=0;
  
  if(query->failed)
    return NULL;

  query_results=rasqal_new_query_results(query);
  if(query_results)
    rasqal_query_add_query_result(query, query_results);


  rc=rasqal_engine_execute_init(query_results);
  if(rc) {
    rasqal_free_query_results(query_results);
    return NULL;
  }

  if(query->factory->execute) {
    rc=query->factory->execute(query, query_results);
    if(rc) {
      rasqal_free_query_results(query_results);
      return NULL;
    }
  }

  query_results->executed=1;

  if(rasqal_engine_execute_order(query_results)) {
    rasqal_free_query_results(query_results);
    query_results=NULL;
  }

  return query_results;
}


static const char* rasqal_query_verb_labels[RASQAL_QUERY_VERB_LAST+1]={
  "Unknown",
  "SELECT",
  "CONSTRUCT",
  "DESCRIBE",
  "ASK"
};

/* Utility methods */

/**
 * rasqal_query_verb_as_string:
 * @verb: the #rasqal_query_verb verb of the query
 *
 * Get a string for the query verb.
 * 
 * Return value: pointer to a shared string label for the query verb
 **/
const char*
rasqal_query_verb_as_string(rasqal_query_verb verb)
{
  if(verb <= RASQAL_QUERY_VERB_UNKNOWN || 
     verb > RASQAL_QUERY_VERB_LAST)
    verb=RASQAL_QUERY_VERB_UNKNOWN;

  return rasqal_query_verb_labels[(int)verb];
}
  

/**
 * rasqal_query_print:
 * @query: the #rasqal_query object
 * @fh: the #FILE* handle to print to.
 *
 * Print a query in a debug format.
 * 
 **/
void
rasqal_query_print(rasqal_query* query, FILE *fh)
{
  fprintf(fh, "query verb: %s\n", rasqal_query_verb_as_string(query->verb));
  
  if(query->distinct)
    fputs("query results distinct: yes\n", fh);
  if(query->limit >= 0)
    fprintf(fh, "query results limit: %d\n", query->limit);
  if(query->offset >= 0)
    fprintf(fh, "query results offset: %d\n", query->offset);

  fputs("data graphs: ", fh);
  if(query->data_graphs)
    raptor_sequence_print(query->data_graphs, fh);
  if(query->variables_sequence) {
    fputs("\nall variables: ", fh); 
    raptor_sequence_print(query->variables_sequence, fh);
  }
  if(query->anon_variables_sequence) {
    fputs("\nanonymous variables: ", fh); 
    raptor_sequence_print(query->anon_variables_sequence, fh);
  }
  if(query->selects) {
    fputs("\nbound variables: ", fh); 
    raptor_sequence_print(query->selects, fh);
  }
  if(query->describes) {
    fputs("\ndescribes: ", fh);
    raptor_sequence_print(query->describes, fh);
  }
  if(query->triples) {
    fputs("\ntriples: ", fh);
    raptor_sequence_print(query->triples, fh);
  }
  if(query->optional_triples) {
    fputs("\noptional triples: ", fh);
    raptor_sequence_print(query->optional_triples, fh);
  }
  if(query->constructs) {
    fputs("\nconstructs: ", fh);
    raptor_sequence_print(query->constructs, fh);
  }
  if(query->prefixes) {
    fputs("\nprefixes: ", fh);
    raptor_sequence_print(query->prefixes, fh);
  }
  if(query->query_graph_pattern) {
    fputs("\nquery graph pattern: ", fh);
    rasqal_graph_pattern_print(query->query_graph_pattern, fh);
  }
  if(query->order_conditions_sequence) {
    fputs("\nquery order conditions: ", fh);
    raptor_sequence_print(query->order_conditions_sequence, fh);
  }
  fputc('\n', fh);
}


static void
rasqal_query_add_query_result(rasqal_query* query,
                              rasqal_query_results* query_results) 
{
  query_results->next=query->results;
  query->results=query_results;
  /* add reference to ensure query lives as long as this runs */
  query->usage++;
}



void
rasqal_query_remove_query_result(rasqal_query* query,
                                 rasqal_query_results* query_results) 
{
  rasqal_query_results *cur, *prev=NULL;
  for(cur=query->results; cur && cur != query_results; cur=cur->next)
    prev=cur;
  
  if(cur == query_results) {
    if(prev)
      prev->next=cur->next;
  }
  if(cur == query->results && cur != NULL)
    query->results=cur->next;

  /* remove reference and free if we are the last */
  rasqal_free_query(query);
}



/**
 * rasqal_query_get_user_data:
 * @query: #rasqal_query
 *
 * Get query user data.
 * 
 * Return value: user data as set by rasqal_query_set_user_data
 **/
void*
rasqal_query_get_user_data(rasqal_query* query)
{
  return query->user_data;
}


/**
 * rasqal_query_set_user_data:
 * @query: #rasqal_query
 * @user_data: some user data to associate with the query
 *
 * Set the query user data.
 *
 **/
void
rasqal_query_set_user_data(rasqal_query* query, void *user_data)
{
  query->user_data=user_data;
}


/**
 * rasqal_query_get_verb:
 * @query: #rasqal_query
 *
 * Get the query verb.
 *
 * Return value: the operating verb of the query of type rasqal_query_verb
 **/
rasqal_query_verb
rasqal_query_get_verb(rasqal_query* query)
{
  return query->verb;
}


/**
 * rasqal_query_get_wildcard:
 * @query: #rasqal_query
 *
 * Get the query verb is wildcard flag.
 *
 * Return value: non-0 if the query verb was a wildcard (such as SELECT *)
 **/
int
rasqal_query_get_wildcard(rasqal_query* query)
{
  return query->wildcard;
}


/**
 * rasqal_query_get_order_conditions_sequence:
 * @query: #rasqal_query query object
 *
 * Get the sequence of query ordering conditions.
 *
 * Return value: a #raptor_sequence of #rasqal_expression pointers.
 **/
raptor_sequence*
rasqal_query_get_order_conditions_sequence(rasqal_query* query)
{
  return query->order_conditions_sequence;
}


/**
 * rasqal_query_get_order_condition:
 * @query: #rasqal_query query object
 * @idx: index into the sequence (0 or larger)
 *
 * Get a query ordering expression in the sequence of query ordering conditions.
 *
 * Return value: a #rasqal_expression pointer or NULL if out of the sequence range
 **/
rasqal_expression*
rasqal_query_get_order_condition(rasqal_query* query, int idx)
{
  if(!query->order_conditions_sequence)
    return NULL;
  
  return (rasqal_expression*)raptor_sequence_get_at(query->order_conditions_sequence, idx);
}


unsigned char*
rasqal_prefix_id(int prefix_id, unsigned char *string)
{
  int tmpid=prefix_id;
  unsigned char* buffer;
  size_t length=strlen((const char*)string)+4;  /* "r" +... + "_" +... \0 */

  while(tmpid/=10)
    length++;
  
  buffer=(unsigned char*)RASQAL_MALLOC(cstring, length);
  if(!buffer)
    return NULL;
  
  sprintf((char*)buffer, "r%d_%s", prefix_id, string);
  
  RASQAL_FREE(cstring, string);
  return buffer;
}


/**
 * rasqal_query_graph_pattern_visit:
 * @query: query
 * @visit_fn: user function to operate on
 * @data: user data to pass to function
 * 
 * Visit all graph patterns in a query with a user function @visit_fn.
 *
 * See also rasqal_graph_pattern_visit().
 **/
void
rasqal_query_graph_pattern_visit(rasqal_query* query, 
                                 rasqal_graph_pattern_visit_fn visit_fn, 
                                 void* data)
{
  rasqal_graph_pattern* gp=rasqal_query_get_query_graph_pattern(query);
  if(!gp)
    return;

  rasqal_graph_pattern_visit(query, gp, visit_fn, data);
}



typedef struct 
{
  raptor_uri* type_uri;
  raptor_uri* base_uri;
  raptor_namespace_stack *nstack;
} sparql_writer_context;


static void
rasqal_query_write_sparql_variable(sparql_writer_context *wc,
                                   raptor_iostream* iostr, rasqal_variable* v)
{
  if(v->type == RASQAL_VARIABLE_TYPE_ANONYMOUS)
    raptor_iostream_write_counted_string(iostr, "_:", 2);
  else
      raptor_iostream_write_byte(iostr, '?');
  raptor_iostream_write_string(iostr, v->name);
}


static void
rasqal_query_write_sparql_uri(sparql_writer_context *wc,
                              raptor_iostream* iostr, raptor_uri* uri)
{
  size_t len;
  unsigned char* string;
  raptor_qname* qname;

  qname=raptor_namespaces_qname_from_uri(wc->nstack, uri, 10);
  if(qname) {
    const raptor_namespace* nspace=raptor_qname_get_namespace(qname);
    if(!raptor_namespace_get_prefix(nspace))
      raptor_iostream_write_byte(iostr, ':');
    raptor_iostream_write_qname(iostr, qname);
    raptor_free_qname(qname);
    return;
  }
  
  if(wc->base_uri)
    string=raptor_uri_to_relative_counted_uri_string(wc->base_uri, uri, &len);
  else
    string=raptor_uri_as_counted_string(uri, &len);

  raptor_iostream_write_byte(iostr, '<');
  raptor_iostream_write_string_ntriples(iostr, string, len, '>');
  raptor_iostream_write_byte(iostr, '>');

  if(wc->base_uri)
    raptor_free_memory(string);
}


static void
rasqal_query_write_sparql_literal(sparql_writer_context *wc,
                                  raptor_iostream* iostr, rasqal_literal* l)
{
  if(!l) {
    raptor_iostream_write_counted_string(iostr, "null", 4);
    return;
  }

  switch(l->type) {
    case RASQAL_LITERAL_URI:
      rasqal_query_write_sparql_uri(wc, iostr, l->value.uri);
      break;
    case RASQAL_LITERAL_BLANK:
      raptor_iostream_write_counted_string(iostr, "_:", 2);
      raptor_iostream_write_string(iostr, l->string);
      break;
    case RASQAL_LITERAL_PATTERN:
      abort();
      break;
    case RASQAL_LITERAL_STRING:
      raptor_iostream_write_byte(iostr, '"');
      raptor_iostream_write_string_ntriples(iostr, l->string, l->string_len, '"');
      raptor_iostream_write_byte(iostr, '"');
      if(l->language) {
        raptor_iostream_write_byte(iostr, '@');
        raptor_iostream_write_string(iostr, l->language);
      }
      if(l->datatype) {
        raptor_iostream_write_counted_string(iostr, "^^", 2);
        rasqal_query_write_sparql_uri(wc, iostr, l->datatype);
      }
      break;
    case RASQAL_LITERAL_QNAME:
      raptor_iostream_write_counted_string(iostr, "QNAME(", 6);
      raptor_iostream_write_counted_string(iostr, l->string, l->string_len);
      raptor_iostream_write_byte(iostr, ')');
      break;
    case RASQAL_LITERAL_INTEGER:
      raptor_iostream_write_decimal(iostr, l->value.integer);
      break;
    case RASQAL_LITERAL_BOOLEAN:
    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
    case RASQAL_LITERAL_DECIMAL:
      raptor_iostream_write_counted_string(iostr, l->string, l->string_len);
      break;
    case RASQAL_LITERAL_VARIABLE:
      rasqal_query_write_sparql_variable(wc, iostr, l->value.variable);
      break;
    case RASQAL_LITERAL_DATETIME:
      raptor_iostream_write_byte(iostr, '"');
      raptor_iostream_write_string_ntriples(iostr, l->string, l->string_len, '"');
      raptor_iostream_write_counted_string(iostr, "\"^^", 3);
      rasqal_query_write_sparql_uri(wc, iostr, rasqal_xsd_datetime_uri);
      break;

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


static void
rasqal_query_write_sparql_triple(sparql_writer_context *wc,
                                 raptor_iostream* iostr, rasqal_triple* triple)
{
  rasqal_query_write_sparql_literal(wc, iostr, triple->subject);
  raptor_iostream_write_byte(iostr, ' ');
  if(triple->predicate->type == RASQAL_LITERAL_URI &&
     raptor_uri_equals(triple->predicate->value.uri, wc->type_uri))
    raptor_iostream_write_byte(iostr, 'a');
  else
    rasqal_query_write_sparql_literal(wc, iostr, triple->predicate);
  raptor_iostream_write_byte(iostr, ' ');
  rasqal_query_write_sparql_literal(wc, iostr, triple->object);
  raptor_iostream_write_counted_string(iostr, " .", 2);
}


#define SPACES_LENGTH 80
static const char spaces[SPACES_LENGTH+1]="                                                                                ";

static void
rasqal_query_write_indent(raptor_iostream* iostr, int indent) 
{
  while(indent > 0) {
    int sp=(indent > SPACES_LENGTH) ? SPACES_LENGTH : indent;
    raptor_iostream_write_bytes(iostr, spaces, sizeof(char), sp);
    indent -= sp;
  }
}

  

static const char* rasqal_sparql_op_labels[RASQAL_EXPR_LAST+1]={
  NULL, /* UNKNOWN */
  "&&",
  "||",
  "=",
  "!=",
  "<",
  ">",
  "<=",
  ">=",
  "-",
  "+",
  "-",
  "*",
  "/",
  NULL, /* REM */
  NULL, /* STR EQ */
  NULL, /* STR NEQ */
  NULL, /* STR_MATCH */
  NULL, /* STR_NMATCH */
  NULL, /* TILDE */
  "!",
  NULL, /* LITERAL */
  NULL, /* FUNCTION */
  "BOUND",
  "STR",
  "LANG",
  "DATATYPE",
  "isIRI",
  "isBLANK",
  "isLITERAL",
  NULL, /* CAST */
  "ASC",
  "DESC",
  "LANGMATCHES",
  "REGEX"
};



static void
rasqal_query_write_sparql_expression_op(sparql_writer_context *wc,
                                        raptor_iostream* iostr,
                                        rasqal_expression* e)
{
  rasqal_op op=e->op;
  const char* string;
  if(op > RASQAL_EXPR_LAST)
    op=RASQAL_EXPR_UNKNOWN;
  string=rasqal_sparql_op_labels[(int)op];
  
  if(string)
    raptor_iostream_write_string(iostr, string);
  else
    raptor_iostream_write_string(iostr, "NONE");
}


static void
rasqal_query_write_sparql_expression(sparql_writer_context *wc,
                                     raptor_iostream* iostr, 
                                     rasqal_expression* e)
{
  int i;
  int count;

  switch(e->op) {
    case RASQAL_EXPR_AND:
    case RASQAL_EXPR_OR:
    case RASQAL_EXPR_EQ:
    case RASQAL_EXPR_NEQ:
    case RASQAL_EXPR_LT:
    case RASQAL_EXPR_GT:
    case RASQAL_EXPR_LE:
    case RASQAL_EXPR_GE:
    case RASQAL_EXPR_PLUS:
    case RASQAL_EXPR_MINUS:
    case RASQAL_EXPR_STAR:
    case RASQAL_EXPR_SLASH:
    case RASQAL_EXPR_REM:
    case RASQAL_EXPR_STR_EQ:
    case RASQAL_EXPR_STR_NEQ:
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg1);
      raptor_iostream_write_byte(iostr, ' ');
      rasqal_query_write_sparql_expression_op(wc, iostr, e);
      raptor_iostream_write_byte(iostr, ' ');
      rasqal_query_write_sparql_expression(wc, iostr, e->arg2);
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;

    case RASQAL_EXPR_BOUND:
    case RASQAL_EXPR_STR:
    case RASQAL_EXPR_LANG:
    case RASQAL_EXPR_DATATYPE:
    case RASQAL_EXPR_ISURI:
    case RASQAL_EXPR_ISBLANK:
    case RASQAL_EXPR_ISLITERAL:
    case RASQAL_EXPR_ORDER_COND_ASC:
    case RASQAL_EXPR_ORDER_COND_DESC:
      rasqal_query_write_sparql_expression_op(wc, iostr, e);
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg1);
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;
      
    case RASQAL_EXPR_LANGMATCHES:
    case RASQAL_EXPR_REGEX:
      rasqal_query_write_sparql_expression_op(wc, iostr, e);
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg1);
      raptor_iostream_write_counted_string(iostr, ", ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg2);
      if(e->op == RASQAL_EXPR_REGEX && e->arg3) {
        raptor_iostream_write_counted_string(iostr, ", ", 2);
        rasqal_query_write_sparql_expression(wc, iostr, e->arg3);
      }
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;

    case RASQAL_EXPR_TILDE:
    case RASQAL_EXPR_BANG:
    case RASQAL_EXPR_UMINUS:
      rasqal_query_write_sparql_expression_op(wc, iostr, e);
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg1);
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;

    case RASQAL_EXPR_LITERAL:
      rasqal_query_write_sparql_literal(wc, iostr, e->literal);
      break;

    case RASQAL_EXPR_FUNCTION:
      raptor_iostream_write_uri(iostr, e->name);
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      count=raptor_sequence_size(e->args);
      for(i=0; i < count ; i++) {
        rasqal_expression* arg=(rasqal_expression*)raptor_sequence_get_at(e->args, i);
        if(i > 0)
          raptor_iostream_write_counted_string(iostr, " ,", 2);
        rasqal_query_write_sparql_expression(wc, iostr, arg);
      }
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;

    case RASQAL_EXPR_CAST:
      raptor_iostream_write_uri(iostr, e->name);
      raptor_iostream_write_counted_string(iostr, "( ", 2);
      rasqal_query_write_sparql_expression(wc, iostr, e->arg1);
      raptor_iostream_write_counted_string(iostr, " )", 2);
      break;

    case RASQAL_EXPR_STR_MATCH:
    case RASQAL_EXPR_STR_NMATCH:
      abort();
      break;

    case RASQAL_EXPR_UNKNOWN:
    default:
      RASQAL_FATAL2("Unknown operation %d", e->op);
  }
}


static void
rasqal_query_write_sparql_graph_pattern(sparql_writer_context *wc,
                                        raptor_iostream* iostr,
                                        rasqal_graph_pattern *gp, 
                                        int gp_index, int indent)
{
  int triple_index=0;
  rasqal_graph_pattern_operator op;
  raptor_sequence *seq;
  
  op=rasqal_graph_pattern_get_operator(gp);
  
  if(op == RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL ||
     op == RASQAL_GRAPH_PATTERN_OPERATOR_GRAPH) {
    /* prefix verbs */
    if(op == RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL) 
      raptor_iostream_write_counted_string(iostr, "OPTIONAL ", 9);
    else {
      raptor_iostream_write_counted_string(iostr, "GRAPH ?", 7);
      raptor_iostream_write_string(iostr, "FIXME");
    }
  }
  raptor_iostream_write_counted_string(iostr, "{\n", 2);

  indent+= 2;

  /* look for triples */
  while(1) {
    rasqal_triple* t=rasqal_graph_pattern_get_triple(gp, triple_index);
    if(!t)
      break;
    
    rasqal_query_write_indent(iostr, indent);
    rasqal_query_write_sparql_triple(wc, iostr, t);
    raptor_iostream_write_byte(iostr, '\n');

    triple_index++;
  }


  /* look for sub-graph patterns */
  seq=rasqal_graph_pattern_get_sub_graph_pattern_sequence(gp);
  if(seq && raptor_sequence_size(seq) > 0) {
    gp_index=0;
    while(1) {
      rasqal_graph_pattern* sgp=rasqal_graph_pattern_get_sub_graph_pattern(gp, gp_index);
      if(!sgp)
        break;
      
      if(!gp_index)
        rasqal_query_write_indent(iostr, indent);
      else {
        if(op == RASQAL_GRAPH_PATTERN_OPERATOR_UNION)
          /* infix verb */
          raptor_iostream_write_counted_string(iostr, " UNION ", 7);
        else {
          /* must be prefix verb */
          raptor_iostream_write_byte(iostr, '\n');
          rasqal_query_write_indent(iostr, indent);
        }
      }
      
      rasqal_query_write_sparql_graph_pattern(wc, iostr, sgp, gp_index, indent);
      
      gp_index++;
    }
    raptor_iostream_write_byte(iostr, '\n');
  }
  

  /* look for constraints */
  seq=rasqal_graph_pattern_get_constraint_sequence(gp);
  if(seq && raptor_sequence_size(seq) > 0) {
    gp_index=0;
    while(1) {
      rasqal_expression* expr=rasqal_graph_pattern_get_constraint(gp, gp_index);
      if(!expr)
        break;
      
      rasqal_query_write_indent(iostr, indent);
      raptor_iostream_write_counted_string(iostr, "FILTER( ", 8);
      rasqal_query_write_sparql_expression(wc, iostr, expr);
      raptor_iostream_write_counted_string(iostr, " )\n", 3);
      
      gp_index++;
    }
  }
  

  indent-=2;
  
  rasqal_query_write_indent(iostr, indent);
  raptor_iostream_write_byte(iostr, '}');
}


    
static int
rasqal_query_write_sparql_20060406(raptor_iostream *iostr, 
                                   rasqal_query* query, raptor_uri *base_uri)
{
  int i;
  raptor_sequence *var_seq=NULL;
  sparql_writer_context wc;
  raptor_uri_handler *uri_handler;
  void *uri_context;
  
  wc.type_uri=raptor_new_uri_for_rdf_concept("type");
  wc.base_uri=NULL;

  raptor_uri_get_handler(&uri_handler, &uri_context);
  wc.nstack=raptor_new_namespaces(uri_handler, uri_context,
                                  (raptor_simple_message_handler)rasqal_query_simple_error,
                                  query,
                                  1);

  if(base_uri) {
    raptor_iostream_write_counted_string(iostr, "BASE ", 5);
    rasqal_query_write_sparql_uri(&wc, iostr, base_uri);
    raptor_iostream_write_byte(iostr, '\n');

    /* from now on all URIs are relative to this */
    wc.base_uri=raptor_uri_copy(base_uri);
  }
  
  
  for(i=0; 1 ; i++) {
    raptor_namespace *nspace;
    rasqal_prefix* p=rasqal_query_get_prefix(query, i);
    if(!p)
      break;
    
    raptor_iostream_write_counted_string(iostr, "PREFIX ", 7);
    if(p->prefix)
      raptor_iostream_write_string(iostr, p->prefix);
    raptor_iostream_write_counted_string(iostr,": ", 2);
    rasqal_query_write_sparql_uri(&wc, iostr, p->uri);
    raptor_iostream_write_byte(iostr, '\n');

    /* Use this constructor so we copy a URI directly */
    nspace=raptor_new_namespace_from_uri(wc.nstack, p->prefix, p->uri, i);
    raptor_namespaces_start_namespace(wc.nstack, nspace);
  }

  raptor_iostream_write_string(iostr,
                               rasqal_query_verb_as_string(query->verb));
  if(query->distinct)
    raptor_iostream_write_counted_string(iostr, " DISTINCT", 9);

  if(query->verb == RASQAL_QUERY_VERB_DESCRIBE)
    var_seq=query->describes;
  else if(query->verb == RASQAL_QUERY_VERB_SELECT)
    var_seq=query->selects;
  
  if(var_seq && query->wildcard)
    raptor_iostream_write_counted_string(iostr, " *", 2);
  else if(var_seq) {
    int count=raptor_sequence_size(var_seq);
    for(i=0; i < count; i++) {
      rasqal_variable* v=(rasqal_variable*)raptor_sequence_get_at(var_seq, i);
      raptor_iostream_write_counted_string(iostr, " ?", 2);
      raptor_iostream_write_string(iostr, v->name);
    }
  }
  raptor_iostream_write_byte(iostr, '\n');

  if(query->data_graphs) {
    for(i=0; 1; i++) {
      rasqal_data_graph* dg=rasqal_query_get_data_graph(query, i);
      if(!dg)
        break;
      
      if(dg->flags & RASQAL_DATA_GRAPH_NAMED)
        continue;
      
      raptor_iostream_write_counted_string(iostr, "FROM ", 5);
      rasqal_query_write_sparql_uri(&wc, iostr, dg->uri);
      raptor_iostream_write_counted_string(iostr, "\n", 1);
    }
    
    for(i=0; 1; i++) {
      rasqal_data_graph* dg=rasqal_query_get_data_graph(query, i);
      if(!dg)
        break;

      if(!(dg->flags & RASQAL_DATA_GRAPH_NAMED))
        continue;
      
      raptor_iostream_write_counted_string(iostr, "FROM NAMED ", 11);
      rasqal_query_write_sparql_uri(&wc, iostr, dg->name_uri);
      raptor_iostream_write_byte(iostr, '\n');
    }
    
  }

  if(query->constructs) {
    raptor_iostream_write_string(iostr, "CONSTRUCT {\n");
    for(i=0; 1; i++) {
      rasqal_triple* t=rasqal_query_get_construct_triple(query, i);
      if(!t)
        break;

      raptor_iostream_write_counted_string(iostr, "  ", 2);
      rasqal_query_write_sparql_triple(&wc, iostr, t);
      raptor_iostream_write_byte(iostr, '\n');
    }
    raptor_iostream_write_counted_string(iostr, "}\n", 2);
  }
  if(query->query_graph_pattern) {
    raptor_iostream_write_counted_string(iostr, "WHERE ", 6);
    rasqal_query_write_sparql_graph_pattern(&wc, iostr,
                                            query->query_graph_pattern, 
                                            -1, 0);
    raptor_iostream_write_byte(iostr, '\n');
  }

  if(query->order_conditions_sequence) {
    raptor_iostream_write_counted_string(iostr, "ORDER BY ", 9);
    for(i=0; 1; i++) {
      rasqal_expression* expr=rasqal_query_get_order_condition(query, i);
      if(!expr)
        break;

      if(i > 0)
        raptor_iostream_write_byte(iostr, ' ');
      rasqal_query_write_sparql_expression(&wc, iostr, expr);
    }
    raptor_iostream_write_byte(iostr, '\n');
  }

  if(query->limit >=0 || query->offset >= 0) {
    if(query->limit >= 0) {
      raptor_iostream_write_counted_string(iostr, "LIMIT ", 7);
      raptor_iostream_write_decimal(iostr, query->limit);
    }
    if(query->offset >= 0) {
      if(query->limit)
        raptor_iostream_write_byte(iostr, ' ');
      raptor_iostream_write_counted_string(iostr, "OFFSET ", 8);
      raptor_iostream_write_decimal(iostr, query->offset);
    }
    raptor_iostream_write_byte(iostr, '\n');
  }

  raptor_free_uri(wc.type_uri);
  if(wc.base_uri)
    raptor_free_uri(wc.base_uri);
  raptor_free_namespaces(wc.nstack);

  return 0;
}


/**
 * rasqal_query_write:
 * @iostr: #raptor_iostream to write the query to
 * @query: #rasqal_query pointer.
 * @format_uri: #raptor_uri describing the format to write (or NULL for default)
 * @base_uri: #raptor_uri base URI of the output format
 *
 * Write a query to an iostream in a specified format.
 * 
 * The supported URIs for the format_uri are:
 *
 * Default: SPARQL Query Language 2006-04-06
 * http://www.w3.org/TR/2006/CR-rdf-sparql-query-20060406/
 *
 * Return value: non-0 on failure
 **/
int
rasqal_query_write(raptor_iostream* iostr, rasqal_query* query,
                   raptor_uri* format_uri, raptor_uri* base_uri)
{
  if(!format_uri ||
     !strcmp((const char*)raptor_uri_as_string(format_uri),
             "http://www.w3.org/TR/rdf-sparql-query/") ||
     !strcmp((const char*)raptor_uri_as_string(format_uri),
             "http://www.w3.org/TR/2006/WD-rdf-sparql-query-20060220/") ||
     !strcmp((const char*)raptor_uri_as_string(format_uri),
             "http://www.w3.org/TR/2006/CR-rdf-sparql-query-20060406/"))
    return rasqal_query_write_sparql_20060406(iostr, query, base_uri);

  return 1;
}


/**
 * rasqal_query_iostream_write_escaped_counted_string:
 * @query: #rasqal_query object
 * @iostr: #raptor_iostream to write the escaped string to
 * @string: string to escape
 * @len: Length of string to escape
 * 
 * Write a string to an iostream in escaped form suitable for the query string.
 * 
 * Return value: non-0 on failure
 **/
int
rasqal_query_iostream_write_escaped_counted_string(rasqal_query* query,
                                                   raptor_iostream* iostr,
                                                   const unsigned char* string,
                                                   size_t len)
{
  if(query->factory->iostream_write_escaped_counted_string)
    return query->factory->iostream_write_escaped_counted_string(query, iostr, 
                                                                 string, len);
  else
    return 1;
}


/**
 * rasqal_query_escape_counted_string:
 * @query: #rasqal_query object
 * @string: string to escape
 * @len: Length of string to escape
 * @output_len_p: Pointer to store length of output string (or NULL)
 * 
 * Convert a string into an escaped form suitable for the query string.
 * 
 * The returned string must be freed by the caller with
 * rasqal_free_memory()
 *
 * Return value: the escaped string or NULL on failure.
 **/
unsigned char*
rasqal_query_escape_counted_string(rasqal_query* query,
                                   const unsigned char* string, 
                                   size_t len,
                                   size_t* output_len_p)
{
  raptor_iostream* iostr;
  void *output_string=NULL;
  int rc;
  
  iostr=raptor_new_iostream_to_string(&output_string, output_len_p,
                                      rasqal_alloc_memory);
  if(!iostr)
    return NULL;
  rc=rasqal_query_iostream_write_escaped_counted_string(query, iostr,
                                                        string, len);
  raptor_free_iostream(iostr);
  if(rc && output_string) {
    rasqal_free_memory(output_string);
    output_string=NULL;
  }
  
  return output_string;
}
