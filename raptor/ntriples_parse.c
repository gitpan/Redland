/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * ntriples_parse.c - Raptor N-Triples Parser implementation
 *
 * $Id: ntriples_parse.c,v 1.61 2003/09/08 12:49:39 cmdjb Exp $
 *
 * N-Triples
 * http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * Copyright (C) 2001-2003 David Beckett - http://purl.org/net/dajobe/
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


/* Prototypes for local functions */
static void raptor_ntriples_generate_statement(raptor_parser *parser, const char *subject, const raptor_ntriples_term_type subject_type, const char *predicate, const raptor_ntriples_term_type predicate_type, const void *object, const raptor_ntriples_term_type object_type, char *object_literal_language, char *object_literal_datatype);

/*
 * NTriples parser object
 */
struct raptor_ntriples_parser_context_s {
  /* current line */
  char *line;
  /* current line length */
  int line_length;
  /* current char in line buffer */
  int offset;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;
};


typedef struct raptor_ntriples_parser_context_s raptor_ntriples_parser_context;



/**
 * raptor_ntriples_parse_init - Initialise the Raptor NTriples parser
 *
 * Return value: non 0 on failure
 **/

static int
raptor_ntriples_parse_init(raptor_parser* rdf_parser, const char *name) {
  return 0;
}


/* PUBLIC FUNCTIONS */


/*
 * raptor_ntriples_parse_terminate - Free the Raptor NTriples parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_ntriples_parse_terminate(raptor_parser *rdf_parser) {
  raptor_ntriples_parser_context *ntriples_parser=(raptor_ntriples_parser_context*)rdf_parser->context;
  if(ntriples_parser->line_length)
    RAPTOR_FREE(cdata, ntriples_parser->line);
}


static const char *term_type_strings[]={
  "URIref",
  "bnodeID",
  "Literal"
};


const char *
raptor_ntriples_term_as_string (raptor_ntriples_term_type term) {
  return term_type_strings[(int)term];
}



static void
raptor_ntriples_generate_statement(raptor_parser *parser, 
                                   const char *subject,
                                   const raptor_ntriples_term_type subject_type,
                                   const char *predicate,
                                   const raptor_ntriples_term_type predicate_type,
                                   const void *object,
                                   const raptor_ntriples_term_type object_type,
                                   char *object_literal_language,
                                   char *object_literal_datatype)
{
  raptor_statement *statement=&parser->statement;
  raptor_uri *subject_uri=NULL;
  int predicate_ordinal=0;
  raptor_uri *predicate_uri=NULL;
  raptor_uri *object_uri=NULL;
  raptor_uri *datatype_uri=NULL;

  /* Two choices for subject from N-Triples */
  if(subject_type == RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE) {
    statement->subject=subject;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else {
    subject_uri=raptor_new_uri_relative_to_base(parser->base_uri, subject);
    statement->subject=subject_uri;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  }

  if(object_literal_datatype)
    datatype_uri=raptor_new_uri_relative_to_base(parser->base_uri, (const char*)object_literal_datatype);

  /* Predicates in N-Triples are URIs or ordinals */
  if(!strncmp(predicate, "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44)) {
    predicate_ordinal=raptor_check_ordinal(predicate+44);
    if(predicate_ordinal > 0) {
      statement->predicate=(void*)&predicate_ordinal;
      statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_ORDINAL;
    } else {
      /* FIXME - could have a warning here */
      /*
      raptor_parser_error(parser, "Illegal rdf:_<n> property in %s.", predicate);
      */
      predicate_ordinal=0;
    }
  }
  
  if(!predicate_ordinal) {
    predicate_uri=raptor_new_uri_relative_to_base(parser->base_uri, predicate);
    statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;
    statement->predicate=predicate_uri;
  }
  

  /* Three choices for object from N-Triples */
  if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_URI_REF) {
    object_uri=raptor_new_uri_relative_to_base(parser->base_uri, (const char*)object);
    statement->object=object_uri;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  } else if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE) {
    statement->object=object;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else { 
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    statement->object=object;
    statement->object_literal_language=(unsigned char*)object_literal_language;
    statement->object_literal_datatype=datatype_uri;
  }

  if(!parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*parser->statement_handler)(parser->user_data, statement);

  if(subject_uri)
    raptor_free_uri(subject_uri);
  if(predicate_uri)
    raptor_free_uri(predicate_uri);
  if(object_uri)
    raptor_free_uri(object_uri);
  if(datatype_uri)
    raptor_free_uri(datatype_uri);
}


/* These are for 7-bit ASCII and not locale-specific */
#define IS_ASCII_ALPHA(c) (((c)>0x40 && (c)<0x5B) || ((c)>0x60 && (c)<0x7B))
#define IS_ASCII_UPPER(c) ((c)>0x40 && (c)<0x5B)
#define IS_ASCII_DIGIT(c) ((c)>0x2F && (c)<0x3A)
#define IS_ASCII_PRINT(c) ((c)>0x1F && (c)<0x7F)
#define TO_ASCII_LOWER(c) ((c)+0x20)

typedef enum {
  RAPTOR_TERM_CLASS_URI,      /* ends on > */
  RAPTOR_TERM_CLASS_BNODEID,  /* ends on first non [A-Za-z][A-Za-z0-9]* */
  RAPTOR_TERM_CLASS_STRING,   /* ends on non-escaped " */
  RAPTOR_TERM_CLASS_LANGUAGE  /* ends on first non [a-z0-9]+ ('-' [a-z0-9]+ )? */
} raptor_ntriples_term_class;


static int 
raptor_ntriples_term_valid(raptor_parser* rdf_parser, 
                           unsigned char c, int position, 
                           raptor_ntriples_term_class class) 
{
  int result=0;

  switch(class) {
    case RAPTOR_TERM_CLASS_URI:
      /* ends on > */
      result=(c!= '>');
      break;
      
    case RAPTOR_TERM_CLASS_BNODEID:  
      /* ends on first non [A-Za-z][A-Za-z0-9]* */
      result=IS_ASCII_ALPHA(c);
      if(position)
        result = (result || IS_ASCII_DIGIT(c));
      break;
      
    case RAPTOR_TERM_CLASS_STRING:
      /* ends on " */
      result=(c!= '"');
      break;

    case RAPTOR_TERM_CLASS_LANGUAGE:
      /* ends on first non [a-z0-9]+ ('-' [a-z0-9]+ )? */
      result=(IS_ASCII_ALPHA(c) || IS_ASCII_DIGIT(c));
      if(position)
        result = (result || c=='-');
      break;
      
    default:
      raptor_parser_fatal_error(rdf_parser, "Unknown ntriples term %d", class);
  }

  return result;
}


/*
 * raptor_ntriples_term - Parse an N-Triples term with escapes
 * @parser: NTriples parser
 * @start: pointer to starting character of string (in)
 * @dest: destination of string (in)
 * @lenp: pointer to length of string (in/out)
 * @dest_lenp: pointer to length of destination string (out)
 * @end_char: string ending character
 * @class: string class
 * 
 * N-Triples strings/URIs are written in ASCII at present; characters
 * outside the printable ASCII range are discarded with a warning.
 * See the grammar for full details of the allowed ranges.
 *
 * Return value: Non 0 on failure
 **/
static int
raptor_ntriples_term(raptor_parser* rdf_parser, 
                     char **start, char *dest, 
                     int *lenp, int *dest_lenp,
                     char end_char,
                     raptor_ntriples_term_class class)
{
  char *p=*start;
  unsigned char c='\0';
  int ulen=0;
  unsigned long unichar=0;
  unsigned int position=0;
  int end_char_seen=0;
  
  /* find end of string, fixing backslashed characters on the way */
  while(*lenp > 0) {
    c = *p;

    p++;
    (*lenp)--;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;

    /* This is an ASCII check, not a printable character check 
     * so isprint() is not appropriate, since that is a locale check.
     */
    if(!IS_ASCII_PRINT(c)) {
      raptor_parser_error(rdf_parser, "Non-printable ASCII character %d (0x%02X) found.", c, c);
      continue;
    }
    
    if(c != '\\') {
      /* finish at non-backslashed end_char */
      if(end_char && c == end_char) {
        end_char_seen=1;
        break;
      }

      if(!raptor_ntriples_term_valid(rdf_parser, c, position, class)) {
        if(end_char) {
          /* end char was expected, so finding an invalid thing is an error */
          raptor_parser_error(rdf_parser, "Missing terminating '%c' (found '%c')", end_char, c);
          return 0;
        } else {
          /* it's the end - so rewind 1 to save next char */
          p--;
          (*lenp)++;
          rdf_parser->locator.column--;
          rdf_parser->locator.byte--;
          break;
        }
      }
      
      /* otherwise store and move on */
      *dest++=c;
      position++;
      continue;
    }

    if(!*lenp) {
      raptor_parser_error(rdf_parser, "\\ at end of line");
      return 0;
    }

    c = *p;

    p++;
    (*lenp)--;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;

    switch(c) {
      case '"':
      case '\\':
        *dest++=c;
        break;
      case 'n':
        *dest++='\n';
        break;
      case 'r':
        *dest++='\r';
        break;
      case 't':
        *dest++='\t';
        break;
      case 'u':
      case 'U':
        ulen=(c == 'u') ? 4 : 8;
        
        if(*lenp < ulen) {
          raptor_parser_error(rdf_parser, "%c over end of line", c);
          return 0;
        }
        
        sscanf(p, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);

        p+=ulen;
        (*lenp)-=ulen;
        rdf_parser->locator.column+=ulen;
        rdf_parser->locator.byte+=ulen;
        
        if(unichar < 0 || unichar > 0x10ffff) {
          raptor_parser_error(rdf_parser, "Illegal Unicode character with code point #x%lX.", unichar);
          break;
        }
          
        dest+=raptor_unicode_char_to_utf8(unichar, dest);
        break;

      default:
        raptor_parser_error(rdf_parser, "Illegal string escape \\%c in \"%s\"", c, start);
        return 0;
    }

    position++;
  } /* end while */

  
  if(end_char && !end_char_seen) {
    raptor_parser_error(rdf_parser, "Missing terminating '%c' before end of line.", end_char);
    return 1;
  }

  /* terminate dest, can be shorter than source */
  *dest='\0';

  if(dest_lenp)
    *dest_lenp=p-*start;

  *start=p;

  return 0;
}


static const char *xml_literal_datatype_uri_string="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral";


static int
raptor_ntriples_parse_line (raptor_parser* rdf_parser, char *buffer, int len) 
{
  int i;
  char *p;
  char *dest;
  char *terms[3];
  int term_lengths[3];
  raptor_ntriples_term_type term_types[3];
  int term_length= 0;
  char *object_literal_language=NULL;
  char *object_literal_datatype=NULL;


  /* ASSERTION:
   * p always points to first char we are considering
   * p[len-1] always points to last char
   */
  
  /* Handle empty  lines */
  if(!len)
    return 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3(raptor_ntriples_parse_line,
                "handling line '%s' (%d bytes)\n", 
                buffer, len);
#endif
  
  p=buffer;

  while(len>0 && isspace((int)*p)) {
    p++;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;
    len--;
  }

  /* Handle empty - all whitespace lines */
  if(!len)
    return 0;
  
  /* Handle comment lines */
  if(*p == '#')
    return 0;
  
  /* Remove trailing spaces */
  while(len>0 && isspace((int)p[len-1])) {
    p[len-1]='\0';
    len--;
  }

  /* can't be empty now - that would have been caught above */
  
  /* Check for terminating '.' */
  if(p[len-1] != '.') {
    /* Move current location to point to problem */
    rdf_parser->locator.column += len-2;
    rdf_parser->locator.byte += len-2;
    raptor_parser_error(rdf_parser, "Missing . at end of line");
    return 0;
  }

  p[len-1]='\0';
  len--;


  /* Must be triple */

  for(i=0; i<3; i++) {
    if(!len) {
      raptor_parser_error(rdf_parser, "Unexpected end of line");
      return 0;
    }
    
    /* Expect either <URI> or _:name */
    if(i == 2) {
      if(*p != '<' && *p != '_' && *p != '"' && *p != 'x') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected <URIref>, _:bnodeID or \"literal\"", *p);
        return 0;
      }
      if(*p == 'x') {
        if(len < 4 || strncmp(p, "xml\"", 4)) {
          raptor_parser_error(rdf_parser, "Saw '%c', expected xml\"...\")", *p);
          return 0;
        }
      }
    } else {
      if(*p != '<' && *p != '_') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected <URIref> or _:bnodeID", *p);
        return 0;
      }
    }

    switch(*p) {
      case '<':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_URI_REF;
        
        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_term(rdf_parser,
                                &p, dest, &len, &term_length, 
                                '>', RAPTOR_TERM_CLASS_URI))
          return 1;
        break;

      case '"':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_LITERAL;
        
        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_term(rdf_parser,
                                &p, dest, &len, &term_length,
                                '"', RAPTOR_TERM_CLASS_STRING))
          return 1;
        
        if(len && (*p == '-' || *p == '@')) {
          if(*p == '-')
            raptor_parser_error(rdf_parser, "Old N-Triples language syntax using \"string\"-lang rather than \"string\"@lang.");

          object_literal_language=p;

          /* Skip - */
          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(!len) {
            raptor_parser_error(rdf_parser, "Missing language after \"string\"-");
            return 0;
          }
          

          if(raptor_ntriples_term(rdf_parser,
                                  &p, object_literal_language, &len, NULL,
                                  '\0', RAPTOR_TERM_CLASS_LANGUAGE))
            return 1;
        }

        if(len >1 && *p == '^' && p[1] == '^') {

          object_literal_datatype=p;

          /* Skip ^^ */
          p+= 2;
          len-= 2;
          rdf_parser->locator.column+= 2;
          rdf_parser->locator.byte+= 2;

          if(!len || (len && *p != '<')) {
            raptor_parser_error(rdf_parser, "Missing datatype URI-ref in\"string\"^^<URI-ref> after ^^");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(raptor_ntriples_term(rdf_parser,
                                  &p, object_literal_datatype, &len, NULL,
                                  '>', RAPTOR_TERM_CLASS_URI))
            return 1;
          
        }

        if(object_literal_datatype && object_literal_language) {
          raptor_parser_error(rdf_parser, "Typed literal used with a language - ignoring the language");
          object_literal_language=NULL;
        }
          

        break;


      case '_':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE;

        /* store where _ was */
        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(!len || (len > 0 && *p != ':')) {
          raptor_parser_error(rdf_parser, "Illegal bNodeID - _ not followed by :");
          return 0;
        }

        /* Found ':' - move on */

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_term(rdf_parser,
                                &p, dest, &len, &term_length,
                                '\0', RAPTOR_TERM_CLASS_BNODEID))
          return 1;

        if(!term_length) {
          raptor_parser_error(rdf_parser, "Bad or missing bNodeID after _:");
          return 0;
        }
        break;

      case 'x':

        raptor_parser_error(rdf_parser, "Old N-Triples XML using xml\"string\"-lang rather than \"string\"@lang^^<http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral>.");

        /* already know we have 'xml"' coming up */
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_LITERAL;
        
        /* 3=strlen("xml") */
        p+=3;
        len-=3;

        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_term(rdf_parser,
                                &p, dest, &len, &term_length, 
                                '"', RAPTOR_TERM_CLASS_STRING))
          return 1;

        /* got XML literal string */
        object_literal_datatype=(char*)xml_literal_datatype_uri_string;

        if(len && (*p == '-' || *p == '@')) {
          if(*p == '-')
            raptor_parser_error(rdf_parser, "Old N-Triples language syntax using xml\"string\"-lang rather than xml\"string\"@lang.");

          object_literal_language=p;

          /* Skip - */
          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(!len) {
            raptor_parser_error(rdf_parser, "Missing language in xml\"string\"-language after -");
            return 0;
          }

          if(raptor_ntriples_term(rdf_parser,
                                  &p, object_literal_language, &len, NULL,
                                  '"', RAPTOR_TERM_CLASS_STRING))
            return 1;
          
        }

        if(len >1 && *p == '^' && p[1] == '^') {

          object_literal_datatype=p;

          /* Skip ^^ */
          p+= 2;
          len-= 2;
          rdf_parser->locator.column+= 2;
          rdf_parser->locator.byte+= 2;

          if(!len || (len && *p != '<')) {
            raptor_parser_error(rdf_parser, "Missing datatype URI-ref in xml\"string\"^^<URI-ref> after ^^");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(raptor_ntriples_term(rdf_parser,
                                  &p, object_literal_datatype, &len, NULL,
                                  '>', RAPTOR_TERM_CLASS_URI))
            return 1;
          
        }

        if(len) {
          if(*p != ' ') {
            raptor_parser_error(rdf_parser, "Missing terminating ' '");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;
        }
        
        break;


      default:
        raptor_parser_fatal_error(rdf_parser, "Unknown term type");
        return 1;
    }


    /* Store term */
    terms[i]=dest; term_lengths[i]=term_length;

    /* Whitespace must separate the terms */
    if(i<2 && !isspace((int)*p)) {
      raptor_parser_error(rdf_parser, "Missing whitespace after term '%s'", terms[i]);
      return 1;
    }

    /* Skip whitespace after terms */
    while(len>0 && isspace((int)*p)) {
      p++;
      len--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr, "item %d: term '%s' len %d type %s\n",
            i, terms[i], term_lengths[i],
            raptor_ntriples_term_as_string(term_types[i]));
#endif    
  }

  if(len) {
    raptor_parser_error(rdf_parser, "Junk before terminating \".\"");
    return 0;
  }
  

  if(object_literal_language) {
    /* Normalize language to lowercase
     * http://www.w3.org/TR/rdf-concepts/#dfn-language-identifier
     */
    for(p=object_literal_language; *p; p++) {
      if(IS_ASCII_UPPER(*p))
        *p=TO_ASCII_LOWER(*p);
    }
  }

  raptor_ntriples_generate_statement(rdf_parser, 
                                     terms[0], term_types[0],
                                     terms[1], term_types[1],
                                     terms[2], term_types[2],
                                     object_literal_language,
                                     object_literal_datatype);

  rdf_parser->locator.byte += len;

  return 0;
}


static int
raptor_ntriples_parse_chunk(raptor_parser* rdf_parser, 
                            const unsigned char *s, size_t len,
                            int is_end)
{
  char *buffer;
  char *ptr;
  char *start;
  char last_nl;
  raptor_ntriples_parser_context *ntriples_parser=(raptor_ntriples_parser_context*)rdf_parser->context;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2(raptor_ntriples_parse_chunk, "adding %d bytes to line buffer\n", len);
#endif

  /* No data?  It's the end */
  if(!len)
    return 0;

  buffer=(char*)RAPTOR_MALLOC(cstring, ntriples_parser->line_length + len + 1);
  if(!buffer) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return 1;
  }

  if(ntriples_parser->line_length) {
    strncpy(buffer, ntriples_parser->line, ntriples_parser->line_length);
    RAPTOR_FREE(cstring, ntriples_parser->line);
  }

  ntriples_parser->line=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+ntriples_parser->line_length;

  /* adjust stored length */
  ntriples_parser->line_length += len;

  /* now write new stuff at end of cdata buffer */
  strncpy(ptr, (char*)s, len);
  ptr += len;
  *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                "line buffer now '%s' (%d bytes)\n", 
                ntriples_parser->line, ntriples_parser->line_length);
#endif

  last_nl='\n';  /* last newline character - \r triggers check */

  ptr=buffer+ntriples_parser->offset;
  start=ptr;
  while(*ptr) {
    /* skip \n when just seen \r - i.e. \r\n or CR LF */
    if(last_nl == '\r' && *ptr == '\n') {
      ptr++;
      rdf_parser->locator.byte++;
    }
    
    while(*ptr && *ptr != '\n' && *ptr != '\r')
      ptr++;

    /* keep going - no newline yet */
    if(!*ptr && !is_end)
      break;

    last_nl=*ptr;

    len=ptr-start;
    rdf_parser->locator.column=0;

    *ptr='\0';
    if(raptor_ntriples_parse_line(rdf_parser,start,len))
      return 1;
    
    rdf_parser->locator.line++;

    /* go past newline */
    ptr++;
    rdf_parser->locator.byte++;

    start=ptr;
  }

  /* exit now, no more input */
  if(is_end)
    return 0;
    
  ntriples_parser->offset=start-buffer;

  len=ntriples_parser->line_length - ntriples_parser->offset;
    
  if(len) {
    /* collapse buffer */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                  "collapsing line buffer from %d to %d bytes\n", 
                  ntriples_parser->line_length, len);
#endif
    buffer=(char*)RAPTOR_MALLOC(cstring, len + 1);
    if(!buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    strncpy(buffer, ntriples_parser->line+ntriples_parser->line_length-len, len);
    buffer[len]='\0';

    RAPTOR_FREE(cstring, ntriples_parser->line);

    ntriples_parser->line=buffer;
    ntriples_parser->line_length -= ntriples_parser->offset;
    ntriples_parser->offset=0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                  "line buffer now '%s' (%d bytes)\n", 
                  ntriples_parser->line, ntriples_parser->line_length);
#endif    
  }
  
  return 0;
}


static int
raptor_ntriples_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;

  locator->line=1;
  locator->column=0;
  locator->byte=0;

  return 0;
}


static void
raptor_ntriples_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_ntriples_parser_context);
  
  factory->init      = raptor_ntriples_parse_init;
  factory->terminate = raptor_ntriples_parse_terminate;
  factory->start     = raptor_ntriples_parse_start;
  factory->chunk     = raptor_ntriples_parse_chunk;
}


void
raptor_init_parser_ntriples (void) {
  raptor_parser_register_factory("ntriples",  "N-Triples",
                                 &raptor_ntriples_parser_register_factory);
}
