/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdump.c - Raptor RDF Parser example code 
 *
 * Copyright (C) 2000-2006, David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>

/* Raptor includes */
#include <raptor.h>

/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <raptor_getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif




#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

static void print_triples(void *user_data, const raptor_statement *statement);
static void print_namespaces(void* user_data, raptor_namespace *nspace);
static void relay_namespaces(void* user_data, raptor_namespace *nspace);
int main(int argc, char *argv[]);


static char *program=NULL;

/* replace newlines in literal string output with spaces */
static int replace_newlines=0;

/* extra noise? */
static int quiet=0;
/* just count, no printing */
static int count=0;

static int triple_count=0;

static raptor_serializer* serializer=NULL;

static int guess=0;

static int reported_guess=0;

static int report_namespace=0;

static
void print_triples(void *user_data, const raptor_statement *triple) 
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;
  triple_count++;

  if(guess && !quiet && !reported_guess) {
     fprintf(stdout, "%s: Guessed parser name '%s'\n", program,
             raptor_get_name(rdf_parser));
     reported_guess=1;
  }


  if(count)
    return;

  /* replace newlines with spaces if object is a literal string */
  if(replace_newlines && 
     triple->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    char *s;
    for(s=(char*)triple->object; *s; s++)
      if(*s == '\n')
        *s=' ';
  }

  raptor_serialize_statement(serializer, triple);
  return;
}


static void
print_namespaces(void* user_data, raptor_namespace *nspace)
{
  /* raptor_parser* rdf_parser=(raptor_parser*)user_data; */
  unsigned char *s;
  
  if(!report_namespace)
    return;

  s=raptor_namespaces_format(nspace, NULL);
  fprintf(stderr, "%s: Namespace declared: %s\n", program, s);
  raptor_free_memory(s);
  
  return;
}


static void
relay_namespaces(void* user_data, raptor_namespace *nspace)
{
  raptor_serializer* rdf_serializer=(raptor_serializer*)user_data;

  if(report_namespace)
    print_namespaces(user_data, nspace);

  raptor_serialize_set_namespace_from_namespace(rdf_serializer, nspace);
}



#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_TEXT_LONG(long, description) "      --" long "  " description
#define HELP_ARG(short, long) "--" #long
#define HELP_PAD "\n                          "
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_TEXT_LONG(long, description)
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "
#endif


#define GETOPT_STRING "nsaf:ghrqo:wecm:i:v"

#ifdef HAVE_GETOPT_LONG
#define SHOW_NAMESPACES_FLAG 0x100

static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"count", 0, 0, 'c'},
  {"ignore-errors", 0, 0, 'e'},
  {"feature", 1, 0, 'f'},
  {"guess", 0, 0, 'g'},
  {"help", 0, 0, 'h'},
  {"input", 1, 0, 'i'},
  {"mode", 1, 0, 'm'},
  {"ntriples", 0, 0, 'n'},
  {"output", 1, 0, 'o'},
  {"quiet", 0, 0, 'q'},
  {"replace-newlines", 0, 0, 'r'},
  {"scan", 0, 0, 's'},
  {"show-namespaces", 0, 0, SHOW_NAMESPACES_FLAG},
  {"ignore-warnings", 0, 0, 'w'},
  {"version", 0, 0, 'v'},
  {NULL, 0, 0, 0}
};
#endif


static int error_count=0;
static int warning_count=0;

static int ignore_warnings=0;
static int ignore_errors=0;

static const char *title_format_string="Raptor RDF syntax parsing and serializing utility %s\n";


static void
rdfdump_error_handler(void *data, raptor_locator *locator,
                      const char *message)
{
  if(!ignore_errors) {
    fprintf(stderr, "%s: Error - ", program);
    raptor_print_locator(stderr, locator);
    fprintf(stderr, " - %s\n", message);
    
    raptor_parse_abort((raptor_parser*)data);
  }

  error_count++;
}


static void
rdfdump_warning_handler(void *data, raptor_locator *locator,
                        const char *message) 
{
  if(!ignore_warnings) {
    fprintf(stderr, "%s: Warning - ", program);
    raptor_print_locator(stderr, locator);
    fprintf(stderr, " - %s\n", message);
  }
  
  warning_count++;
}

struct namespace_decl
{
  unsigned char *prefix;
  unsigned char *uri_string;
};


static void
rdfdump_free_namespace_decl(void* data) {
  struct namespace_decl* nsd=(struct namespace_decl*)data;
  if(nsd->prefix)
    raptor_free_memory(nsd->prefix);
  if(nsd->uri_string)
    raptor_free_memory(nsd->uri_string);
  raptor_free_memory(nsd);
}


int
main(int argc, char *argv[]) 
{
  raptor_parser* rdf_parser=NULL;
  unsigned char *uri_string=NULL;
  int free_uri_string=0;
  unsigned char *base_uri_string=NULL;
  int rc;
  int scanning=0;
  const char *syntax_name="rdfxml";
  const char *serializer_syntax_name="ntriples";
  int strict_mode=0;
  int usage=0;
  int help=0;
  raptor_uri *base_uri;
  raptor_uri *uri;
  char *p;
  char *filename=NULL;
  raptor_feature parser_feature=(raptor_feature)-1;
  int parser_feature_value= -1;
  unsigned char* parser_feature_string_value=NULL;
  raptor_feature serializer_feature=(raptor_feature)-1;
  int serializer_feature_value= -1;
  unsigned char* serializer_feature_string_value=NULL;
  raptor_sequence *namespace_declarations=NULL;

  program=argv[0];
  if((p=strrchr(program, '/')))
    program=p+1;
  else if((p=strrchr(program, '\\')))
    program=p+1;
  argv[0]=program;

  raptor_init();

  while (!usage && !help)
  {
    int c;
#ifdef HAVE_GETOPT_LONG
    int option_index = 0;

    c = getopt_long (argc, argv, GETOPT_STRING, long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING);
#endif
    if (c == -1)
      break;

    switch (c) {
      case 0:
      case '?': /* getopt() - unknown option */
        usage=1;
        break;
        
      case 'a':
        break;

      case 'c':
        count=1;
        break;

      case 'f':
        if(optarg) {
          if(!strcmp(optarg, "help")) {
            int i;
            
            fprintf(stderr, "%s: Valid parser features are:\n", program);
            for(i=0; i < (int)raptor_get_feature_count(); i++) {
              const char *feature_name;
              const char *feature_label;
              if(!raptor_features_enumerate((raptor_feature)i, &feature_name, NULL, &feature_label)) {
                const char *feature_type=(raptor_feature_value_type((raptor_feature)i) == 0) ? "" : " (string)";
                printf("  %-20s  %s%s\n", feature_name, feature_label, 
                       feature_type);
              }
            }
            fprintf(stderr, "%s: Valid serializer features are:\n", program);
            for(i=0; i < (int)raptor_get_feature_count(); i++) {
              const char *feature_name;
              const char *feature_label;
              if(!raptor_serializer_features_enumerate((raptor_feature)i, &feature_name, NULL, &feature_label)) {
                const char *feature_type=(raptor_feature_value_type((raptor_feature)i) == 0) ? "" : " (string)";
                printf("  %-20s  %s%s\n", feature_name, feature_label, 
                       feature_type);
              }
            }
            fputs("Features are set with `" HELP_ARG(f, feature) " FEATURE=VALUE or `-f FEATURE'\nand take a decimal integer VALUE except where noted, defaulting to 1 if omitted.\n", stderr);
            fputs("\nA feature of the form xmlns:PREFIX=\"URI\" can be used to declare output\nnamespace prefixes and names for serializing using an XML-style syntax\nEither or both of PREFIX or URI can be omitted such as -f xmlns=\"URI\"\nThis form can be repeated for multiple declarations.\n", stderr);

            raptor_finish();

            exit(0);
          } else if(!strncmp(optarg, "xmlns", 5)) {
            struct namespace_decl *nd;
            nd=(struct namespace_decl *)raptor_alloc_memory(sizeof(struct namespace_decl));
            if(raptor_new_namespace_parts_from_string((unsigned char*)optarg,
                                                      &nd->prefix,
                                                      &nd->uri_string)) {
              fprintf(stderr, "%s: Bad xmlns syntax in '%s'\n", program, 
                      optarg);
              rdfdump_free_namespace_decl(nd);
              exit(0);
            }

            if(!namespace_declarations)
              namespace_declarations=raptor_new_sequence(rdfdump_free_namespace_decl, NULL);

            raptor_sequence_push(namespace_declarations, nd);
          } else {
            int i;
            size_t arg_len=strlen(optarg);
            
            for(i=0; i < (int)raptor_get_feature_count(); i++) {
              const char *feature_name;
              size_t len;
              
              if(raptor_features_enumerate((raptor_feature)i, &feature_name, NULL, NULL))
                continue;
              len=strlen(feature_name);
              if(!strncmp(optarg, feature_name, len)) {
                parser_feature=(raptor_feature)i;
                if(raptor_feature_value_type(parser_feature) == 0) {
                  if(len < arg_len && optarg[len] == '=')
                    parser_feature_value=atoi(&optarg[len+1]);
                  else if(len == arg_len)
                    parser_feature_value=1;
                } else {
                  if(len < arg_len && optarg[len] == '=')
                    parser_feature_string_value=(unsigned char*)&optarg[len+1];
                  else if(len == arg_len)
                    parser_feature_string_value=(unsigned char*)"";
                }
                break;
              }
            }
            
            for(i=0; i < (int)raptor_get_feature_count(); i++) {
              const char *feature_name;
              size_t len;
              
              if(raptor_serializer_features_enumerate((raptor_feature)i, &feature_name, NULL, NULL))
                continue;
              len=strlen(feature_name);
              if(!strncmp(optarg, feature_name, len)) {
                serializer_feature=(raptor_feature)i;
                if(raptor_feature_value_type(serializer_feature) == 0) {
                  if(len < arg_len && optarg[len] == '=')
                    serializer_feature_value=atoi(&optarg[len+1]);
                  else if(len == arg_len)
                    serializer_feature_value=1;
                } else {
                  if(len < arg_len && optarg[len] == '=')
                    serializer_feature_string_value=(unsigned char*)&optarg[len+1];
                  else if(len == arg_len)
                    serializer_feature_string_value=(unsigned char*)"";
                }
                break;
              }
            }
            
            if(parser_feature_value < 0 && !parser_feature_string_value &&
               serializer_feature_value < 0 && !serializer_feature_string_value) {
              fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(f, feature) "'\nTry '%s " HELP_ARG(f, feature) " help' for a list of valid features\n",
                      program, optarg, program);
              usage=1;
            }
          }
        }
        break;

      case 'g':
        guess=1;
        break;

      case 'h':
        help=1;
        break;

      case 'n':
        syntax_name="ntriples";
        break;

      case 's':
        scanning=1;
        break;

      case 'q':
        quiet=1;
        break;

      case 'r':
        replace_newlines=1;
        break;

      case 'm':
        if(optarg) {
          if(!strcmp(optarg, "strict"))
            strict_mode=1;
          else if (!strcmp(optarg, "lax"))
            strict_mode=0;
          else {
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(m, mode) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n  - `lax'\n  - `strict'\n");
            usage=1;
          }
        }
        break;

      case 'o':
        if(optarg) {
          if(raptor_serializer_syntax_name_check(optarg))
            serializer_syntax_name=optarg;
          else {
            int i;
            
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i=0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage=1;
            break;
            
          }
        }
        break;

      case 'i':
        if(optarg) {
          if(raptor_syntax_name_check(optarg))
            syntax_name=optarg;
          else {
            int i;
            
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(i, input) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i=0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_syntaxes_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage=1;
            break;
            
          }
        }
        break;

      case 'w':
        ignore_warnings=1;
        break;
        
      case 'e':
        ignore_errors=1;
        break;

      case 'v':
        fputs(raptor_version_string, stdout);
        fputc('\n', stdout);

        raptor_finish();

        exit(0);

#ifdef SHOW_NAMESPACES_FLAG
      case SHOW_NAMESPACES_FLAG:
        report_namespace=1;
        break;
#endif

    }
    
  }

  if(optind != argc-1 && optind != argc-2 && !help && !usage) {
    usage=2; /* Title and usage */
  }

  
  if(usage) {
    if(usage>1) {
      fprintf(stderr, title_format_string, raptor_version_string);
      fputs(raptor_short_copyright_string, stderr);
      fputc('\n', stderr);
    }
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
                    program);
    raptor_finish();

    exit(1);
  }

  if(help) {
    int i;
    
    printf("Usage: %s [OPTIONS] <source URI> [base URI]\n", program);
    printf(title_format_string, raptor_version_string);
    puts(raptor_short_copyright_string);
    puts("Parse RDF content at the source URI into RDF triples.");
    puts("\nSyntax options:");
    puts(HELP_TEXT("i FORMAT", "input FORMAT ", "Set the input format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_syntaxes_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      printf("    %-14s          %s", help_name, help_label);
      if(!i)
        puts(" (default)");
      else
        putchar('\n');
    }
    puts(HELP_TEXT("o FORMAT", "output FORMAT", "Set the output format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      printf("    %-14s          %s", help_name, help_label);
      if(!i)
        puts(" (default)");
      else
        putchar('\n');
    }
    puts("\nGeneral options:");
    puts(HELP_TEXT("c", "count           ", "Count triples only - do not print them."));
    puts(HELP_TEXT("e", "ignore-errors   ", "Ignore error messages"));
    puts(HELP_TEXT("f FEATURE(=VALUE)", "feature FEATURE(=VALUE)", HELP_PAD "Set parser or serializer features" HELP_PAD "Use `-f help' for a list of valid features"));
    puts(HELP_TEXT("g", "guess           ", "Guess the input syntax (same as -i guess)"));
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("m MODE", "mode MODE  ", "Set parser mode - 'lax' (default) or 'strict'"));
    puts(HELP_TEXT("q", "quiet           ", "No extra information messages"));
    puts(HELP_TEXT("r", "replace-newlines", "Replace newlines with spaces in literals"));
    puts(HELP_TEXT("s", "scan            ", "Scan for <rdf:RDF> element in source"));
#ifdef SHOW_NAMESPACES_FLAG
    puts(HELP_TEXT_LONG("show-namespaces ", "Show namespaces as they are declared"));
#endif
    puts(HELP_TEXT("w", "ignore-warnings ", "Ignore warning messages"));
    puts(HELP_TEXT("v", "version         ", "Print the Raptor version"));
    puts("\nReport bugs to http://bugs.librdf.org/");
    puts("Raptor home page: http://librdf.org/raptor/");

    raptor_finish();

    exit(0);
  }


  if(optind == argc-1)
    uri_string=(unsigned char*)argv[optind];
  else {
    uri_string=(unsigned char*)argv[optind++];
    base_uri_string=(unsigned char*)argv[optind];
  }

  /* If uri_string is "path-to-file", turn it into a file: URI */
  if(!strcmp((const char*)uri_string, "-")) {
    if(!base_uri_string) {
      fprintf(stderr, "%s: A Base URI is required when reading from standard input.\n",
              program);
      return(1);
    }
    uri_string=NULL;
  } else if(!access((const char*)uri_string, R_OK)) {
    filename=(char*)uri_string;
    uri_string=raptor_uri_filename_to_uri_string(filename);
    if(!uri_string) {
      fprintf(stderr, "%s: Failed to create URI for file %s.\n",
              program, filename);
      return(1);
    }
    free_uri_string=1;
  }

  if(uri_string) {
    uri=raptor_new_uri(uri_string);
    if(!uri) {
      fprintf(stderr, "%s: Failed to create URI for %s\n",
              program, uri_string);
      return(1);
    }
  } else
    uri=NULL; /* stdin */


  if(!base_uri_string) {
    base_uri=raptor_uri_copy(uri);
  } else {
    base_uri=raptor_new_uri(base_uri_string);
    if(!base_uri) {
      fprintf(stderr, "%s: Failed to create URI for %s\n",
              program, base_uri_string);
      return(1);
    }
  }

  if(guess)
    syntax_name="guess";

  rdf_parser=raptor_new_parser(syntax_name);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n", program,
            syntax_name);
    return(1);
  }

  raptor_set_error_handler(rdf_parser, rdf_parser, rdfdump_error_handler);
  raptor_set_warning_handler(rdf_parser, rdf_parser, rdfdump_warning_handler);
  
  raptor_set_parser_strict(rdf_parser, strict_mode);
  
  if(scanning)
    raptor_set_feature(rdf_parser, RAPTOR_FEATURE_SCANNING, 1);

  if(parser_feature_value >= 0)
    raptor_set_feature(rdf_parser, 
                       parser_feature, parser_feature_value);
  if(parser_feature_string_value)
    raptor_parser_set_feature_string(rdf_parser, 
                                     parser_feature,
                                     parser_feature_string_value);


  if(!quiet) {
    if (filename) {
      if(base_uri_string)
        fprintf(stdout, "%s: Parsing file %s with base URI %s\n", program,
                filename, base_uri_string);
      else
        fprintf(stdout, "%s: Parsing file %s\n", program, filename);
    } else {
      if(base_uri_string)
        fprintf(stdout, "%s: Parsing URI %s with base URI %s\n", program,
                uri_string, base_uri_string);
      else
        fprintf(stdout, "%s: Parsing URI %s\n", program, uri_string);
    }
  }
  
  raptor_set_statement_handler(rdf_parser, rdf_parser, print_triples);

  if(report_namespace)
    raptor_set_namespace_handler(rdf_parser, rdf_parser, print_namespaces);


  if(serializer_syntax_name) {    
    serializer=raptor_new_serializer(serializer_syntax_name);
    if(!serializer) {
      fprintf(stderr, 
              "%s: Failed to create raptor serializer type %s\n", program,
              serializer_syntax_name);
      return(1);
    }

    if(namespace_declarations) {
      int i;
      for(i=0; i< raptor_sequence_size(namespace_declarations); i++) {
        struct namespace_decl *nd=(struct namespace_decl *)raptor_sequence_get_at(namespace_declarations, i);
        raptor_uri *ns_uri=NULL;
        if(nd->uri_string)
          ns_uri=raptor_new_uri(nd->uri_string);
        
        raptor_serialize_set_namespace(serializer, ns_uri, nd->prefix);
        if(ns_uri)
          raptor_free_uri(ns_uri);
      }
      raptor_free_sequence(namespace_declarations);
      namespace_declarations=NULL;
    }
    
    if(serializer_feature_value >= 0)
      raptor_serializer_set_feature(serializer, 
                                    serializer_feature, serializer_feature_value);
    if(serializer_feature_string_value)
      raptor_serializer_set_feature_string(serializer, 
                                           serializer_feature,
                                           serializer_feature_string_value);

    raptor_serialize_start_to_file_handle(serializer, base_uri, stdout);

    if(!report_namespace)
      raptor_set_namespace_handler(rdf_parser, serializer, relay_namespaces);
  }
  

  /* PARSE the URI as RDF/XML */
  rc=0;
  if(!uri || filename) {
    if(raptor_parse_file(rdf_parser, uri, base_uri)) {
      fprintf(stderr, "%s: Failed to parse file %s %s content\n", program, 
              filename, syntax_name);
      rc=1;
    }
  } else {
    if(raptor_parse_uri(rdf_parser, uri, base_uri)) {
      fprintf(stderr, "%s: Failed to parse URI %s %s content\n", program, 
              uri_string, syntax_name);
      rc=1;
    }
  }

  raptor_free_parser(rdf_parser);

  if(serializer) {
    raptor_serialize_end(serializer);
    raptor_free_serializer(serializer);
  }
  

  if(!quiet)
    fprintf(stdout, "%s: Parsing returned %d triple%s\n", program,
            triple_count, (triple_count == 1 ? "" : "s"));

  raptor_free_uri(base_uri);
  if(uri)
    raptor_free_uri(uri);
  if(free_uri_string)
    raptor_free_memory(uri_string);

  if(namespace_declarations)
    raptor_free_sequence(namespace_declarations);

  raptor_finish();

  if(error_count && !ignore_errors)
    return 1;

  if(warning_count && !ignore_warnings)
    return 2;

  return(rc);
}
