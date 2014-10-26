/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal_literal.c - Rasqal literals
 *
 * $Id: rasqal_literal.c 11551 2006-10-29 21:12:27Z dajobe $
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
/* for isnan() */
#include <math.h>

#ifdef RASQAL_REGEX_PCRE
#include <pcre.h>
#endif

#ifdef RASQAL_REGEX_POSIX
#include <sys/types.h>
#include <regex.h>
#endif

#include "rasqal.h"
#include "rasqal_internal.h"



/**
 * rasqal_new_integer_literal:
 * @type: Type of literal such as RASQAL_LITERAL_INTEGER or RASQAL_LITERAL_BOOLEAN
 * @integer: int value
 *
 * Constructor - Create a new Rasqal integer literal.
 * 
 * The integer decimal number is turned into a rasqal integer literal
 * and given a datatype of xsd:integer
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_integer_literal(rasqal_literal_type type, int integer)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=type;
  l->value.integer=integer;
  l->string=(unsigned char*)RASQAL_MALLOC(cstring, 30); /* FIXME */
  sprintf((char*)l->string, "%d", integer);
  l->string_len=strlen((const char*)l->string);
  l->datatype=raptor_uri_copy(rasqal_xsd_integer_uri);
  l->usage=1;
  return l;
}


/**
 * rasqal_new_double_literal:
 * @d: double literal
 *
 * Constructor - Create a new Rasqal double literal.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_double_literal(double d)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=RASQAL_LITERAL_DOUBLE;
  l->value.floating=d;
  l->string=(unsigned char*)RASQAL_MALLOC(cstring, 30); /* FIXME */
  sprintf((char*)l->string, "%1g", d);
  l->string_len=strlen((const char*)l->string);
  l->datatype=raptor_uri_copy(rasqal_xsd_double_uri);
  l->usage=1;
  return l;
}


/**
 * rasqal_new_floating_literal:
 * @f:  floating literal
 * 
 * @Deprecated: Use rasqal_new_double_literal().
 *
 * Constructor - Create a new Rasqal double literal.
 *
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_floating_literal(double f)
{
  return rasqal_new_double_literal(f);
}


/**
 * rasqal_new_uri_literal:
 * @uri: #raptor_uri uri
 *
 * Constructor - Create a new Rasqal URI literal from a raptor URI.
 *
 * The uri is an input parameter and is stored in the literal, not copied.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_uri_literal(raptor_uri *uri)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=RASQAL_LITERAL_URI;
  l->value.uri=uri;
  l->usage=1;
  return l;
}


/**
 * rasqal_new_pattern_literal:
 * @pattern: regex pattern
 * @flags: regex flags
 *
 * Constructor - Create a new Rasqal pattern literal.
 *
 * The pattern and flags are input parameters and are stored in the
 * literal, not copied.  The set of flags recognised depends
 * on the regex engine and the query language.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_pattern_literal(const unsigned char *pattern, 
                           const char *flags)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=RASQAL_LITERAL_PATTERN;
  l->string=pattern;
  l->string_len=strlen((const char*)pattern);
  l->flags=(const unsigned char*)flags;
  l->usage=1;
  return l;
}


/**
 * rasqal_new_decimal_literal:
 * @decimal: decimal literal
 *
 * Constructor - Create a new Rasqal decimal literal.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_decimal_literal(const unsigned char *decimal)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=RASQAL_LITERAL_DECIMAL;
  l->string_len=strlen((const char*)decimal);
  l->string=(unsigned char*)RASQAL_MALLOC(cstring, l->string_len+1);
  strcpy((char*)l->string, (const char*)decimal);
  l->datatype=raptor_uri_copy(rasqal_xsd_decimal_uri);
  l->usage=1;
  return l;
}


/*
 * rasqal_literal_string_to_native - INTERNAL Upgrade a datatyped literal string to an internal typed literal
 * @l: #rasqal_literal to operate on inline
 * @error_handler: error handling function
 * @error_data: data for error handle
 *
 * At present this promotes datatyped literals
 * xsd:integer to RASQAL_LITERAL_INTEGER
 * xsd:double to RASQAL_LITERAL_DOUBLE
 * xsd:float to RASQAL_LITERAL_FLOAT
 * xsd:boolean to RASQAL_LITERAL_BOOLEAN
 *
 * Return value: non-0 on failure
 **/
int
rasqal_literal_string_to_native(rasqal_literal *l,
                                raptor_simple_message_handler error_handler,
                                void *error_data)
{
  if(!l->datatype)
    return 0;

  if(raptor_uri_equals(l->datatype, rasqal_xsd_integer_uri)) {
    int i=atoi((const char*)l->string);

    if(l->language) {
      RASQAL_FREE(cstring, (void*)l->language);
      l->language=NULL;
    }

    l->type=RASQAL_LITERAL_INTEGER;
    l->value.integer=i;
    return 0;
  }
  
  if(raptor_uri_equals(l->datatype, rasqal_xsd_double_uri) ||
     raptor_uri_equals(l->datatype, rasqal_xsd_float_uri)) {
    double d=0.0;
    int n;

    n=sscanf((char*)l->string, "%lf", &d);
    if(n != 1) {
      if(error_handler)
        error_handler(error_data, "Illegal floating point string '%s'",
                      l->string);
      return 1;
    }

    if(l->language) {
      RASQAL_FREE(cstring, (void*)l->language);
      l->language=NULL;
    }

    l->type= raptor_uri_equals(l->datatype, rasqal_xsd_float_uri) ?
      RASQAL_LITERAL_FLOAT : RASQAL_LITERAL_DOUBLE;
    l->value.floating=d;
    return 0;
  }

  if(raptor_uri_equals(l->datatype, rasqal_xsd_boolean_uri)) {
    int b=0;
    if(!strcmp((const char*)l->string, "true") || 
       !strcmp((const char*)l->string, "TRUE") ||
       !strcmp((const char*)l->string, "1"))
       b=1;
    
    if(l->language) {
      RASQAL_FREE(cstring, (void*)l->language);
      l->language=NULL;
    }

    /* static string for boolean */
    l->string=b ? RASQAL_XSD_BOOLEAN_TRUE : RASQAL_XSD_BOOLEAN_FALSE;
    l->string_len=(b ? 4 : 5);

    l->type=RASQAL_LITERAL_BOOLEAN;
    l->value.integer=b;
    return 0;
  }

  return 0;
}


/**
 * rasqal_new_string_literal:
 * @string: UTF-8 string lexical form
 * @language: RDF language (xml:lang) (or NULL)
 * @datatype: datatype URI (or NULL)
 * @datatype_qname: datatype qname string (or NULL)
 *
 * Constructor - Create a new Rasqal string literal.
 * 
 * All parameters are input parameters and if present are stored in
 * the literal, not copied.
 * 
 * The datatype and datatype_qname parameters are alternatives; the
 * qname is a datatype that cannot be resolved till later since the
 * prefixes have not yet been declared or checked.
 * 
 * If the string literal is datatyped and of certain types recognised
 * it may be converted to a different literal type by
 * rasqal_literal_string_to_native.
 *
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_string_literal(const unsigned char *string,
                          const char *language,
                          raptor_uri *datatype, 
                          const unsigned char *datatype_qname)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  if(datatype && language) {
    RASQAL_FREE(cstring, (void*)language);
    language=NULL;
  }

  l->type=RASQAL_LITERAL_STRING;
  l->string=string;
  l->string_len=strlen((const char*)string);
  l->language=language;
  l->datatype=datatype;
  l->flags=datatype_qname;
  l->usage=1;

  if(rasqal_literal_string_to_native(l, NULL, NULL)) {
    rasqal_free_literal(l);
    l=NULL;
  }
    
  return l;
}


/**
 * rasqal_new_simple_literal:
 * @type: RASQAL_LITERAL_BLANK or RASQAL_LITERAL_BLANK_QNAME
 * @string: the UTF-8 string value to store
 *
 * Constructor - Create a new Rasqal simple literal.
 * 
 * The string is an input parameter and is stored in the
 * literal, not copied.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_simple_literal(rasqal_literal_type type, 
                          const unsigned char *string)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=type;
  l->string=string;
  l->string_len=strlen((const char*)string);
  l->usage=1;
  return l;
}


/**
 * rasqal_new_boolean_literal:
 * @value: non-0 for true, 0 for false
 *
 * Constructor - Create a new Rasqal boolean literal.
 *
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_boolean_literal(int value)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

  l->type=RASQAL_LITERAL_BOOLEAN;
  l->value.integer=value;
  l->string=value ? RASQAL_XSD_BOOLEAN_TRUE : RASQAL_XSD_BOOLEAN_FALSE;
  l->string_len=(value ? 4 : 5);
  l->usage=1;
  return l;
}


/**
 * rasqal_new_variable_literal:
 * @variable: #rasqal_variable to use
 *
 * Constructor - Create a new Rasqal variable literal.
 * 
 * variable is an input parameter and stored in the literal, not copied.
 * 
 * Return value: New #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_new_variable_literal(rasqal_variable *variable)
{
  rasqal_literal* l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));
  l->type=RASQAL_LITERAL_VARIABLE;
  l->value.variable=variable;
  l->usage=1;
  return l;
}


/**
 * rasqal_new_literal_from_literal:
 * @l: #rasqal_literal object to copy
 *
 * Copy Constructor - create a new rasqal_literal object from an existing rasqal_literal object.
 * 
 * Return value: a new #rasqal_literal object or NULL on failure
 **/
rasqal_literal*
rasqal_new_literal_from_literal(rasqal_literal* l)
{
  l->usage++;
  return l;
}


/**
 * rasqal_free_literal:
 * @l: #rasqal_literal object
 *
 * Destructor - destroy an rasqal_literal object.
 * 
 **/
void
rasqal_free_literal(rasqal_literal* l)
{
  if(--l->usage)
    return;
  
  switch(l->type) {
    case RASQAL_LITERAL_URI:
      if(l->value.uri)
        raptor_free_uri(l->value.uri);
      break;
    case RASQAL_LITERAL_STRING:
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_INTEGER: 
    case RASQAL_LITERAL_FLOAT:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
     if(l->string)
        RASQAL_FREE(cstring, (void*)l->string);
      if(l->language)
        RASQAL_FREE(cstring, (void*)l->language);
      if(l->datatype)
        raptor_free_uri(l->datatype);
      if(l->type == RASQAL_LITERAL_STRING ||
         l->type == RASQAL_LITERAL_PATTERN) {
        if(l->flags)
          RASQAL_FREE(cstring, (void*)l->flags);
      }
      break;

    case RASQAL_LITERAL_BOOLEAN:
      /* static l->string for boolean, does not need freeing */
      break;

    case RASQAL_LITERAL_VARIABLE:
      /* It is correct that this is not called here
       * since all variables are shared and owned by
       * the rasqal_query sequence variables_sequence */

      /* rasqal_free_variable(l->value.variable); */
      break;

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
  RASQAL_FREE(rasqal_literal, l);
}


/* 
 * The order here must match that of rasqal_literal_type
 * in rasqal.h and is significant as rasqal_literal_compare
 * uses it for type comparisons with the RASQAL_COMPARE_XQUERY
 * flag.
 */
static const char* rasqal_literal_type_labels[RASQAL_LITERAL_LAST+1]={
  "UNKNOWN",
  "blank",
  "uri",
  "string",
  "boolean",
  "integer",
  "double",
  "float",
  "decimal",
  "datetime",
  "pattern",
  "qname",
  "variable"
};


/**
 * rasqal_literal_print_type:
 * @l: the #rasqal_literal object
 * @fh: the #FILE* handle to print to
 * 
 * Print a string form for a rasqal literal type.
 *
 **/
void
rasqal_literal_print_type(rasqal_literal* l, FILE* fh)
{
  rasqal_literal_type type;

  if(!l) {
    fputs("null", fh);
    return;
  }
  
  type=l->type;
  if(type > RASQAL_LITERAL_LAST)
    type=RASQAL_LITERAL_UNKNOWN;
  fputs(rasqal_literal_type_labels[(int)type], fh);
}


/**
 * rasqal_literal_print:
 * @l: the #rasqal_literal object
 * @fh: the #FILE* handle to print to
 *
 * Print a Rasqal literal in a debug format.
 * 
 * The print debug format may change in any release.
 **/
void
rasqal_literal_print(rasqal_literal* l, FILE* fh)
{
  if(!l) {
    fputs("null", fh);
    return;
  }

  if(l->type != RASQAL_LITERAL_VARIABLE)
    rasqal_literal_print_type(l, fh);

  switch(l->type) {
    case RASQAL_LITERAL_URI:
      fprintf(fh, "<%s>", raptor_uri_as_string(l->value.uri));
      break;
    case RASQAL_LITERAL_BLANK:
      fprintf(fh, " %s", l->string);
      break;
    case RASQAL_LITERAL_PATTERN:
      fprintf(fh, "/%s/%s", l->string, l->flags ? (const char*)l->flags : "");
      break;
    case RASQAL_LITERAL_STRING:
      fputs("(\"", fh);
      raptor_print_ntriples_string(fh, l->string, '"');
      fputc('"', fh);
      if(l->language)
        fprintf(fh, "@%s", l->language);
      if(l->datatype)
        fprintf(fh, "^^<%s>", raptor_uri_as_string(l->datatype));
      fputc(')', fh);
      break;
    case RASQAL_LITERAL_QNAME:
      fprintf(fh, "(%s)", l->string);
      break;
    case RASQAL_LITERAL_INTEGER:
      fprintf(fh, " %d", l->value.integer);
      break;
    case RASQAL_LITERAL_BOOLEAN:
      fprintf(fh, "(%s)", l->string);
      break;
    case RASQAL_LITERAL_DOUBLE:
      fprintf(fh, " %g", l->value.floating);
      break;
    case RASQAL_LITERAL_VARIABLE:
      rasqal_variable_print(l->value.variable, fh);
      break;
    case RASQAL_LITERAL_FLOAT:
      fprintf(fh, " float(%g)", l->value.floating);
      break;
    case RASQAL_LITERAL_DECIMAL:
      fprintf(fh, " decimal(%s)", l->string);
      break;
    case RASQAL_LITERAL_DATETIME:
      fprintf(fh, " datetime(%s)", l->string);
      break;

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}



/*
 * rasqal_literal_as_boolean - INTERNAL Return a literal as a boolean value
 * @l: #rasqal_literal object
 * @error: pointer to error flag
 * 
 * Literals are true if not NULL (uris, strings) or zero (0, 0.0).
 * Otherwise the error flag is set.
 * 
 * Return value: non-0 if true
 **/
int
rasqal_literal_as_boolean(rasqal_literal* l, int *error)
{
  if(!l)
    return 0;
  
  switch(l->type) {
    case RASQAL_LITERAL_URI:
      return (l->value.uri) != NULL;
      break;
      
    case RASQAL_LITERAL_STRING:
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      return (l->string) != NULL;
      break;

    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
      return l->value.integer != 0;
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
      return l->value.floating != 0.0;
      break;

    case RASQAL_LITERAL_VARIABLE:
      return rasqal_literal_as_boolean(l->value.variable->value, error);
      break;

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


/*
 * rasqal_literal_as_integer - INTERNAL Return a literal as an integer value
 * @l: #rasqal_literal object
 * @error: pointer to error flag
 * 
 * Integers, booleans, double and float literals natural are turned into
 * integers. If string values are the lexical form of an integer, that is
 * returned.  Otherwise the error flag is set.
 * 
 * Return value: integer value
 **/
int
rasqal_literal_as_integer(rasqal_literal* l, int *error)
{
  if(!l)
    return 0;
  
  switch(l->type) {
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
      return l->value.integer != 0;
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
      return (int)l->value.floating;
      break;

    case RASQAL_LITERAL_STRING:
      {
        char *eptr;
        double  d;
        int v;

        eptr=NULL;
        v=(int)strtol((const char*)l->string, &eptr, 10);
        if((unsigned char*)eptr != l->string && *eptr=='\0')
          return v;

        eptr=NULL;
        d=strtod((const char*)l->string, &eptr);
        if((unsigned char*)eptr != l->string && *eptr=='\0')
          return (int)d;
      }
      *error=1;
      return 0;
      break;

    case RASQAL_LITERAL_VARIABLE:
      return rasqal_literal_as_integer(l->value.variable->value, error);
      break;

    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_URI:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      *error=1;
      return 0;
      
    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


/*
 * rasqal_literal_as_floating - INTERNAL Return a literal as a floating value
 * @l: #rasqal_literal object
 * @error: pointer to error flag
 * 
 * Integers, booleans, double and float literals natural are turned into
 * integers. If string values are the lexical form of an floating, that is
 * returned.  Otherwise the error flag is set.
 * 
 * Return value: floating value
 **/
double
rasqal_literal_as_floating(rasqal_literal* l, int *error)
{
  if(!l)
    return 0;
  
  switch(l->type) {
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
      return (double)l->value.integer;
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
      return l->value.floating;
      break;

    case RASQAL_LITERAL_STRING:
      {
        char *eptr=NULL;
        double  d=strtod((const char*)l->string, &eptr);
        if((unsigned char*)eptr != l->string && *eptr=='\0')
          return d;
      }
      *error=1;
      return 0.0;
      break;

    case RASQAL_LITERAL_VARIABLE:
      return rasqal_literal_as_integer(l->value.variable->value, error);
      break;

    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_URI:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      *error=1;
      return 0.0;
      
    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


/*
 * rasqal_literal_as_uri - INTERNAL Return a literal as a raptor_uri*
 * @l: #rasqal_literal object
 * 
 * Return value: raptor_uri* value or NULL on failure
 **/
raptor_uri*
rasqal_literal_as_uri(rasqal_literal* l)
{
  if(!l)
    return NULL;
  
  if(l->type==RASQAL_LITERAL_URI)
    return l->value.uri;

  if(l->type==RASQAL_LITERAL_VARIABLE)
    return rasqal_literal_as_uri(l->value.variable->value);

  abort();

  return NULL;
}


/**
 * rasqal_literal_as_string_flags:
 * @l: #rasqal_literal object
 * @flags: comparison flags
 * @error: pointer to error
 *
 * Return the string format of a literal according to flags.
 * 
 * flag bits affects conversion:
 *   RASQAL_COMPARE_XQUERY: use XQuery conversion rules
 * 
 * If @error is not NULL, *error is set to non-0 on error
 *
 * Return value: pointer to a shared string format of the literal.
 **/
const unsigned char*
rasqal_literal_as_string_flags(rasqal_literal* l, int flags, int *error)
{
  if(!l)
    return NULL;
  
  switch(l->type) {
    case RASQAL_LITERAL_BOOLEAN:
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_STRING:
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_FLOAT:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      return l->string;

    case RASQAL_LITERAL_URI:
      if(flags & RASQAL_COMPARE_XQUERY) {
        if(error)
          *error=1;
        return NULL;
      }
      return raptor_uri_as_string(l->value.uri);

    case RASQAL_LITERAL_VARIABLE:
      return rasqal_literal_as_string_flags(l->value.variable->value, flags,
                                            error);

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


/**
 * rasqal_literal_as_string:
 * @l: #rasqal_literal object
 *
 * Return the string format of a literal.
 * 
 * Return value: pointer to a shared string format of the literal.
 **/
const unsigned char*
rasqal_literal_as_string(rasqal_literal* l)
{
  return rasqal_literal_as_string_flags(l, 0, NULL);
}

/**
 * rasqal_literal_as_variable:
 * @l: #rasqal_literal object
 *
 * Get the variable inside a literal.
 * 
 * Return value: the #rasqal_variable or NULL if the literal is not a variable
 **/
rasqal_variable*
rasqal_literal_as_variable(rasqal_literal* l)
{
  return (l->type == RASQAL_LITERAL_VARIABLE) ? l->value.variable : NULL;
}


/* turn the sign of the double into an int, for comparison purposes */
static RASQAL_INLINE int
double_to_int(double d) 
{
  if(d == 0.0)
    return 0;
  return (d < 0.0) ? -1 : 1;
}


/**
 * rasqal_literal_compare:
 * @l1: #rasqal_literal first literal
 * @l2: #rasqal_literal second literal
 * @flags: comparison flags
 * @error: pointer to error
 *
 * Compare two literals with type promotion.
 * 
 * The two literals are compared across their range.  If the types
 * are not the same, they are promoted.  If one is a double or float, the
 * other is promoted to double, otherwise for integers, otherwise
 * to strings (all literals have a string value).
 *
 * The comparison returned is as for strcmp, first before second
 * returns <0.  equal returns 0, and first after second returns >0.
 * For URIs, the string value is used for the comparsion.
 *
 * flag bits affects comparisons:
 *   RASQAL_COMPARE_NOCASE: use case independent string comparisons
 *   RASQAL_COMPARE_XQUERY: use XQuery comparison and type promotion rules
 * 
 * If @error is not NULL, *error is set to non-0 on error
 *
 * Return value: <0, 0, or >0 as described above.
 **/
int
rasqal_literal_compare(rasqal_literal* l1, rasqal_literal* l2, int flags,
                       int *error)
{
  rasqal_literal *lits[2];
  unsigned int type;
  int i;
  int ints[2];
  double doubles[2];
  const unsigned char* strings[2];
  int errori=0;
  int seen_string=0;
  int seen_int=0;
  int seen_double=0;
  int seen_boolean=0;
  int seen_numeric=0;
  
  *error=0;

  /* null literals */
  if(!l1 || !l2) {
    /* if either is not null, the comparison fails */
    if(l1 || l2)
      *error=1;
    return 0;
  }

  lits[0]=l1;  lits[1]=l2;
  for(i=0; i<2; i++) {
    if(lits[i]->type == RASQAL_LITERAL_VARIABLE) {
      lits[i]=lits[i]->value.variable->value;

      /* Need to re-check for NULL values */
      if(!lits[i]) {
        /* A null value, so the comparison fails */
        RASQAL_DEBUG2("literal %d is a variable with no value\n", i);
        if(lits[1-i])
          *error=1;
        return 0;
      }

      RASQAL_DEBUG3("literal %d is a variable, value is a %s\n", i,
                    rasqal_literal_type_labels[lits[i]->type]);

    }
    

    switch(lits[i]->type) {
      case RASQAL_LITERAL_URI:
        break;

      case RASQAL_LITERAL_DECIMAL:
        seen_numeric++;
        strings[i]=lits[i]->string;
        break;

      case RASQAL_LITERAL_STRING:
      case RASQAL_LITERAL_BLANK:
      case RASQAL_LITERAL_PATTERN:
      case RASQAL_LITERAL_QNAME:
      case RASQAL_LITERAL_DATETIME:
        strings[i]=lits[i]->string;
        seen_string++;
        break;

      case RASQAL_LITERAL_BOOLEAN:
        seen_boolean=1;
        ints[i]=lits[i]->value.integer;
        break;
        
      case RASQAL_LITERAL_INTEGER:
        ints[i]=lits[i]->value.integer;
        seen_int++;
        seen_numeric++;
        break;
    
      case RASQAL_LITERAL_DOUBLE:
      case RASQAL_LITERAL_FLOAT:
        doubles[i]=lits[i]->value.floating;
        seen_double++;
        seen_numeric++;
        break;

      case RASQAL_LITERAL_VARIABLE:
        /* this case was dealt with above, retrieving the value */
        
      case RASQAL_LITERAL_UNKNOWN:
      default:
        abort();
    }
  } /* end for i=0,1 */


  /* work out type to aim for */
  if(lits[0]->type != lits[1]->type) {
    RASQAL_DEBUG3("literal 0 type %s.  literal 1 type %s\n", 
                  rasqal_literal_type_labels[lits[0]->type],
                  rasqal_literal_type_labels[lits[1]->type]);

    if(flags & RASQAL_COMPARE_XQUERY) { 
      int type0=(int)lits[0]->type;
      int type1=(int)lits[1]->type;
      RASQAL_DEBUG3("xquery literal compare types %d vs %d\n", type0, type1);
      if(seen_numeric != 2) {
        return type0 - type1;
      }
      /* FIXME - promote all numeric to double or int for now */
      type=seen_double ? RASQAL_LITERAL_DOUBLE : RASQAL_LITERAL_INTEGER;
    } else {
      type=seen_string ? RASQAL_LITERAL_STRING : RASQAL_LITERAL_INTEGER;
      if((seen_int & seen_double) || (seen_int & seen_string))
        type=RASQAL_LITERAL_DOUBLE;
      if(seen_boolean & seen_string)
        type=RASQAL_LITERAL_STRING;
    }
  } else
    type=lits[0]->type;
  

  /* do promotions */
  for(i=0; i<2; i++ ) {
    if(lits[i]->type == type)
      continue;
    
    switch(type) {
      case RASQAL_LITERAL_DOUBLE:
        doubles[i]=rasqal_literal_as_floating(lits[i], &errori);
        /* failure always means no match */
        if(errori)
          return 1;
        RASQAL_DEBUG4("promoted literal %d (type %s) to a floating, with value %g\n", 
                      i, rasqal_literal_type_labels[lits[i]->type], doubles[i]);
        break;

      case RASQAL_LITERAL_INTEGER:
        ints[i]=rasqal_literal_as_integer(lits[i], &errori);
        /* failure always means no match */
        if(errori)
          return 1;
        RASQAL_DEBUG4("promoted literal %d (type %s) to an integer, with value %d\n", 
                      i, rasqal_literal_type_labels[lits[i]->type], ints[i]);
        break;
    
      case RASQAL_LITERAL_STRING:
       strings[i]=rasqal_literal_as_string(lits[i]);
       RASQAL_DEBUG4("promoted literal %d (type %s) to a string, with value '%s'\n", 
                     i, rasqal_literal_type_labels[lits[i]->type], strings[i]);
       break;

      case RASQAL_LITERAL_BOOLEAN:
        ints[i]=rasqal_literal_as_boolean(lits[i], &errori);
        /* failure always means no match */
        if(errori)
          return 1;
        RASQAL_DEBUG4("promoted literal %d (type %s) to a boolean, with value %d\n", 
                      i, rasqal_literal_type_labels[lits[i]->type], ints[i]);
        break;
    
      default:
        *error=1;
        return 0;
    }

  } /* check types are promoted */
  

  switch(type) {
    case RASQAL_LITERAL_URI:
      return strcmp((const char*)raptor_uri_as_string(lits[0]->value.uri),
                    (const char*)raptor_uri_as_string(lits[1]->value.uri));

    case RASQAL_LITERAL_STRING:
      if(lits[0]->language || lits[1]->language) {
        /* if either is null, the comparison fails */
        if(!lits[0]->language || !lits[1]->language)
          return 1;
        if(rasqal_strcasecmp(lits[0]->language,lits[1]->language))
          return 1;
      }

      if(lits[0]->datatype || lits[1]->datatype) {
        int result;
        
        /* if either is NULL, do not compare */
        if(!lits[0]->datatype || !lits[1]->datatype)
          return lits[0]->datatype ? 1 : -1;

        result=strcmp((const char*)raptor_uri_as_string(lits[0]->datatype),
                      (const char*)raptor_uri_as_string(lits[1]->datatype));

        if(result)
          return result;
      }
      
      /* FALLTHROUGH */
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      if(flags & RASQAL_COMPARE_NOCASE)
        return rasqal_strcasecmp((const char*)strings[0], (const char*)strings[1]);
      else
        return strcmp((const char*)strings[0], (const char*)strings[1]);

    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
      return ints[0] - ints[1];
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
      return double_to_int(doubles[0] - doubles[1]);
      break;

    default:
      abort();
  }
}


/**
 * rasqal_literal_equals:
 * @l1: #rasqal_literal literal
 * @l2: #rasqal_literal data literal
 *
 * Compare two literals with no type promotion.
 * 
 * If the l2 data literal value is a boolean, it will match
 * the string "true" or "false" in the first literal l1.
 *
 * Return value: non-0 if equal
 **/
int
rasqal_literal_equals(rasqal_literal* l1, rasqal_literal* l2)
{
  /* null literals */
  if(!l1 || !l2) {
    /* if either is not null, the comparison fails */
    return (l1 || l2);
  }

  if(l1->type != l2->type) {
    if(l2->type == RASQAL_LITERAL_BOOLEAN &&
       l1->type == RASQAL_LITERAL_STRING)
      return !strcmp((const char*)l1->string, (const char*)l2->string);
    return 0;
  }
  
  switch(l1->type) {
    case RASQAL_LITERAL_URI:
      return raptor_uri_equals(l1->value.uri, l2->value.uri);

    case RASQAL_LITERAL_STRING:
      if(l1->language || l2->language) {
        /* if either is null, the comparison fails */
        if(!l1->language || !l2->language)
          return 0;
        if(rasqal_strcasecmp(l1->language,l2->language))
          return 0;
      }

      if(l1->datatype || l2->datatype) {
        /* if either is null, the comparison fails */
        if(!l1->datatype || !l2->datatype)
          return 0;
        if(!raptor_uri_equals(l1->datatype,l2->datatype))
          return 0;
      }
      
      /* FALLTHROUGH */
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      return !strcmp((const char*)l1->string, (const char*)l2->string);
      break;
      
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
      return l1->value.integer == l2->value.integer;
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
      return l1->value.floating == l2->value.floating;
      break;

    case RASQAL_LITERAL_VARIABLE:
      /* both are variables */
      return rasqal_literal_equals(l1->value.variable->value,
                                   l2->value.variable->value);
      
    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


/*
 * rasqal_literal_expand_qname - INTERNAL Expand any qname in a literal into a URI
 * @user_data: #rasqal_query cast as void for use with raptor_sequence_foreach
 * @l: #rasqal_literal literal
 * 
 * Expands any QName inside the literal using prefixes that are
 * declared in the query that may not have been present when the
 * literal was first declared.  Intended to be used standalone
 * as well as with raptor_sequence_foreach which takes a function
 * signature that this function matches.
 * 
 * Return value: non-0 on failure
 **/
int
rasqal_literal_expand_qname(void *user_data, rasqal_literal *l)
{
  rasqal_query *rq=(rasqal_query *)user_data;

  if(l->type == RASQAL_LITERAL_QNAME) {
    /* expand a literal qname */
    raptor_uri *uri=raptor_qname_string_to_uri(rq->namespaces,
                                               l->string, l->string_len,
                                               (raptor_simple_message_handler)rasqal_query_simple_error, rq);
    if(!uri)
      return 1;
    RASQAL_FREE(cstring, (void*)l->string);
    l->string=NULL;
    l->type=RASQAL_LITERAL_URI;
    l->value.uri=uri;
  } else if (l->type == RASQAL_LITERAL_STRING) {
    raptor_uri *uri;
    
    if(l->flags) {
      /* expand a literal string datatype qname */
      uri=raptor_qname_string_to_uri(rq->namespaces,
                                     l->flags, 
                                     strlen((const char*)l->flags),
                                     (raptor_simple_message_handler)rasqal_query_simple_error, rq);
      if(!uri)
        return 1;
      l->datatype=uri;
      RASQAL_FREE(cstring, (void*)l->flags);
      l->flags=NULL;

      if(l->language && uri) {
        RASQAL_FREE(cstring, (void*)l->language);
        l->language=NULL;
      }

      if(rasqal_literal_string_to_native(l, (raptor_simple_message_handler)rasqal_query_simple_error, rq)) {
        rasqal_free_literal(l);
        return 1;
      }
    }
  }
  return 0;
}


/*
 * rasqal_literal_has_qname - INTERNAL Check if literal has a qname part
 * @l: #rasqal_literal literal
 * 
 * Checks if any part ofthe literal has an unexpanded QName.
 * 
 * Return value: non-0 if a QName is present
 **/
int
rasqal_literal_has_qname(rasqal_literal *l) {
  return (l->type == RASQAL_LITERAL_QNAME) ||
         (l->type == RASQAL_LITERAL_STRING && (l->flags));
}


/**
 * rasqal_literal_as_node:
 * @l: #rasqal_literal object
 *
 * Turn a literal into a new RDF string, URI or blank literal.
 * 
 * Return value: the new #rasqal_literal or NULL on failure
 **/
rasqal_literal*
rasqal_literal_as_node(rasqal_literal* l)
{
  raptor_uri *dt_uri=NULL;
  rasqal_literal* new_l;
  
  switch(l->type) {
    case RASQAL_LITERAL_URI:
    case RASQAL_LITERAL_STRING:
    case RASQAL_LITERAL_BLANK:
      new_l=rasqal_new_literal_from_literal(l);
      break;
      
    case RASQAL_LITERAL_VARIABLE:
      new_l=l->value.variable->value;
      if(new_l)
        return rasqal_new_literal_from_literal(new_l);
      else
        return NULL;
      break;

    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      if(l->type == RASQAL_LITERAL_BOOLEAN)
        dt_uri=raptor_uri_copy(rasqal_xsd_boolean_uri);
      else
        dt_uri=raptor_uri_copy(l->datatype);

      new_l=(rasqal_literal*)RASQAL_CALLOC(rasqal_literal, 1, sizeof(rasqal_literal));

      new_l->type=RASQAL_LITERAL_STRING;
      new_l->string_len=strlen((const char*)l->string);
      new_l->string=(unsigned char*)RASQAL_MALLOC(cstring, new_l->string_len+1);
      strcpy((char*)new_l->string, (const char*)l->string);
      new_l->datatype=dt_uri;
      new_l->flags=NULL;
      new_l->usage=1;
      break;
      
    case RASQAL_LITERAL_QNAME:
      /* QNames should be gone by the time expression eval happens */

    case RASQAL_LITERAL_PATTERN:
      /* FALLTHROUGH */

    case RASQAL_LITERAL_UNKNOWN:
    default:
      RASQAL_FATAL2("Cannot turn literal type %d into a node", l->type);
      abort();
  }
  
  return new_l;
}


/*
 * rasqal_literal_ebv - INTERNAL Get the rasqal_literal effective boolean value
 * @l: #rasqal_literal literal
 * 
 * Return value: non-0 if EBV is true, else false
 **/
int
rasqal_literal_ebv(rasqal_literal* l) 
{
  rasqal_variable* v;
  /* Result is true unless... */
  int b=1;
  
  v=rasqal_literal_as_variable(l);
  if(v) {
    if(v->value == NULL) {
      /* ... The operand is unbound */
      b=0;
      goto done;
    }
    l=v->value;
  }
  
  if(l->type == RASQAL_LITERAL_BOOLEAN && !l->value.integer) {
    /* ... The operand is an xs:boolean with a FALSE value. */
    b=0;
  } else if(l->type == RASQAL_LITERAL_STRING && 
            !l->datatype && !l->string_len) {
    /* ... The operand is a 0-length untyped RDF literal or xs:string. */
    b=0;
  } else if((l->type == RASQAL_LITERAL_INTEGER && !l->value.integer) ||
            ((l->type == RASQAL_LITERAL_DOUBLE || 
              l->type == RASQAL_LITERAL_FLOAT) &&
             !l->value.floating)
            ) {
    /* ... The operand is any numeric type with a value of 0. */
    /* FIXME - deal with decimal */
    b=0;
  } else if((l->type == RASQAL_LITERAL_DOUBLE || 
             l->type == RASQAL_LITERAL_FLOAT) &&
            isnan(l->value.floating)
            ) {
    /* ... The operand is an xs:double or xs:float with a value of NaN */
    b=0;
  }
  
  done:
  return b;
}


/*
 * rasqal_literal_Is_constant - INTERNAL Check if a literal is a constant
 * @l: #rasqal_literal literal
 * 
 * Return value: non-0 if literal is a constant
 **/
int
rasqal_literal_is_constant(rasqal_literal* l)
{
  switch(l->type) {
    case RASQAL_LITERAL_URI:
    case RASQAL_LITERAL_BLANK:
    case RASQAL_LITERAL_STRING:
    case RASQAL_LITERAL_PATTERN:
    case RASQAL_LITERAL_QNAME:
    case RASQAL_LITERAL_INTEGER:
    case RASQAL_LITERAL_BOOLEAN:
    case RASQAL_LITERAL_DOUBLE:
    case RASQAL_LITERAL_FLOAT:
    case RASQAL_LITERAL_DECIMAL:
    case RASQAL_LITERAL_DATETIME:
      return 1;

    case RASQAL_LITERAL_VARIABLE:
      return 0;

    case RASQAL_LITERAL_UNKNOWN:
    default:
      abort();
  }
}


rasqal_formula*
rasqal_new_formula(void) 
{
  return (rasqal_formula*)RASQAL_CALLOC(rasqal_formula, 1, sizeof(rasqal_formula));
}

void
rasqal_free_formula(rasqal_formula* formula)
{
  if(formula->triples)
    raptor_free_sequence(formula->triples);
  if(formula->value)
    rasqal_free_literal(formula->value);
  RASQAL_FREE(rasqal_formula, formula);
}
  

void
rasqal_formula_print(rasqal_formula* formula, FILE *stream)
{
  fputs("formula(triples=", stream);
  if(formula->triples)
    raptor_sequence_print(formula->triples, stream);
  else
    fputs("[]", stream);
  fputs(", value=", stream);
  if(formula->value)
    rasqal_literal_print(formula->value, stream);
  else
    fputs("NULL", stream);
  fputc(')', stream);
}


rasqal_formula*
rasqal_formula_join(rasqal_formula* first_formula, 
                    rasqal_formula* second_formula)
{
  if(!first_formula && !second_formula)
    return NULL;

  if(!first_formula)
    return second_formula;
  
  if(!second_formula)
    return first_formula;
  
  if(first_formula->triples || second_formula->triples) {
    if(!first_formula->triples) {
      first_formula->triples=second_formula->triples;
      second_formula->triples=NULL;
    } else if(second_formula->triples)
      raptor_sequence_join(first_formula->triples, second_formula->triples);
    
    rasqal_free_formula(second_formula);
  }
  
  return first_formula;
}

