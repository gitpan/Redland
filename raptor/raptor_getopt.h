/* 
 * Public Domain getopt header
 *
 * $Id: raptor_getopt.h,v 1.3 2003/02/19 11:33:11 cmdjb Exp $
 *
 */

#ifndef RAPTOR_GETOPT_H
#define RAPTOR_GETOPT_H

int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind, opterr, optopt;

#endif
