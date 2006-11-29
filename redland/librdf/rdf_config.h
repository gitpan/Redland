/* librdf/rdf_config.h.  Generated from rdf_config.h.in by configure.  */
/* librdf/rdf_config.h.in.  Generated from configure.ac by autoheader.  */

/* BDB has close method with 2 args */
#define HAVE_BDB_CLOSE_2_ARGS 1

/* BDB defines DBC */
#define HAVE_BDB_CURSOR 1

/* BDB cursor method has 4 arguments */
#define HAVE_BDB_CURSOR_4_ARGS 1

/* BDB defines DB_TXN */
#define HAVE_BDB_DB_TXN 1

/* BDB has fd method with 2 args */
#define HAVE_BDB_FD_2_ARGS 1

/* Have BDB hash support */
#define HAVE_BDB_HASH 1

/* BDB has open method with 6 args */
/* #undef HAVE_BDB_OPEN_6_ARGS */

/* BDB has open method with 7 args */
#define HAVE_BDB_OPEN_7_ARGS 1

/* BDB has set_flags method */
#define HAVE_BDB_SET_FLAGS 1

/* BDB has dbopen method */
/* #undef HAVE_DBOPEN */

/* BDB has db_create method */
#define HAVE_DB_CREATE 1

/* Define to 1 if you have the <db.h> header file. */
#define HAVE_DB_H 1

/* BDB has db_open method */
/* #undef HAVE_DB_OPEN */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <dmalloc.h> header file. */
/* #undef HAVE_DMALLOC_H */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getenv' function. */
#define HAVE_GETENV 1

/* Define to 1 if you have the `getopt' function. */
#define HAVE_GETOPT 1

/* Define to 1 if you have the <getopt.h> header file. */
#define HAVE_GETOPT_H 1

/* Define to 1 if you have the `getopt_long' function. */
#define HAVE_GETOPT_LONG 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libpq-fe.h> header file. */
#define HAVE_LIBPQ_FE_H 1

/* Have local MD5 digest */
/* #undef HAVE_LOCAL_MD5_DIGEST */

/* Have local RIPEMD160 digest */
/* #undef HAVE_LOCAL_RIPEMD160_DIGEST */

/* Have local SHA1 digest */
/* #undef HAVE_LOCAL_SHA1_DIGEST */

/* Define to 1 if you have the `memcmp' function. */
#define HAVE_MEMCMP 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkstemp' function. */
#define HAVE_MKSTEMP 1

/* Define to 1 if you have the `mktemp' function. */
#define HAVE_MKTEMP 1

/* Define to 1 if you have the <openssl/crypto.h> header file. */
#define HAVE_OPENSSL_CRYPTO_H 1

/* Have openssl MD5 digest */
#define HAVE_OPENSSL_CRYPTO_MD5_DIGEST 1

/* Have openssl RIPEMD160 digest */
#define HAVE_OPENSSL_CRYPTO_RIPEMD160_DIGEST 1

/* Have openssl SHA1 digest */
#define HAVE_OPENSSL_CRYPTO_SHA1_DIGEST 1

/* Have openssl digests */
#define HAVE_OPENSSL_DIGESTS 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Have Raptor RDF parser */
#define HAVE_RAPTOR_RDF_PARSER 1

/* Define to 1 if you have the <sqlite3.h> header file. */
#define HAVE_SQLITE3_H 1

/* Define to 1 if you have the <sqlite.h> header file. */
/* #undef HAVE_SQLITE_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the `tmpnam' function. */
#define HAVE_TMPNAM 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Run time assertion checks. */
#define LIBRDF_ASSERT 1

/* Print run time assertion check failure messages. */
#define LIBRDF_ASSERT_MESSAGES 1

/* Release version as a decimal */
#define LIBRDF_VERSION_DECIMAL 10005

/* Major version number */
#define LIBRDF_VERSION_MAJOR 1

/* Minor version number */
#define LIBRDF_VERSION_MINOR 0

/* Release version number */
#define LIBRDF_VERSION_RELEASE 5

/* Name of package */
#define PACKAGE "redland"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://bugs.librdf.org/"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Redland RDF Application Framework"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Redland RDF Application Framework 1.0.5"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "redland"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.5"

/* The size of `unsigned char', as computed by sizeof. */
#define SIZEOF_UNSIGNED_CHAR 1

/* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* The size of `unsigned long long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* SQLite API version */
#define SQLITE_API 3

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Building file storage */
#define STORAGE_FILE 1

/* Building hashes storage */
#define STORAGE_HASHES 1

/* Building memory storage */
#define STORAGE_MEMORY 1

/* Building MySQL storage */
#define STORAGE_MYSQL 1

/* Building PostgreSQL storage */
#define STORAGE_POSTGRESQL 1

/* Building SQLite storage */
#define STORAGE_SQLITE 1

/* Building 3store storage */
/* #undef STORAGE_TSTORE */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Version number of package */
#define VERSION "1.0.5"

/* Use POSIX threads */
/* #undef WITH_THREADS */

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */
