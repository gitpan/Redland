/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdf_hash_bdb.h - RDF DB Berkeley DB Interface definition
 *
 * $Id: rdf_hash_bdb.h,v 1.8 2001/02/18 23:59:47 cmdjb Exp $
 *
 * Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
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



#ifndef LIBRDF_HASH_BDB_H
#define LIBRDF_HASH_BDB_H

#ifdef __cplusplus
extern "C" {
#endif


void librdf_init_hash_bdb(librdf_world *world);


#ifdef __cplusplus
}
#endif

#endif
