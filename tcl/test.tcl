#
# Test code for Redland Tcl interface
#
# $Id: test.tcl,v 1.12 2003/08/27 20:47:09 cmdjb Exp $
#
# Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.org/
# University of Bristol - http://www.bristol.ac.uk/
# 
# This package is Free Software or Open Source available under the
# following licenses (these are alternatives):
#   1. GNU Lesser General Public License (LGPL)
#   2. GNU General Public License (GPL)
#   3. Mozilla Public License (MPL)
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# full license terms.
# 
# 
#


lappend auto_path .

package require redland

set world [librdf_new_world]
librdf_world_open $world

set storage [librdf_new_storage $world "hashes" "test" {new='yes',hash-type='bdb',dir='.'}]
if {"$storage" == "NULL"} then {
  error "Failed to create RDF storage"
}

set model [librdf_new_model $world $storage ""]
if {"$model" == "NULL"} then {
  librdf_free_storage $storage
  error "Failed to create RDF model"
}


set statement [librdf_new_statement_from_nodes $world [librdf_new_node_from_uri_string $world "http://purl.org/net/dajobe/"] [librdf_new_node_from_uri_string $world "http://purl.org/dc/elements/1.1/creator"] [librdf_new_node_from_literal $world "Dave Beckett" "" 0]]
if {"$statement" == "NULL"} then {
  librdf_free_model $model
  librdf_free_storage $storage
  error "failed to create RDF statement"
}

librdf_model_add_statement $model $statement


# Match against an empty statement - find everything
set statement [librdf_new_statement_from_nodes $world NULL NULL NULL]

# after this statement should not be touched since find_statements is using it
set stream [librdf_model_find_statements $model $statement]

while {! [librdf_stream_end $stream]} {
  set statement2 [librdf_stream_get_object $stream]
  puts [concat "found statement:" [librdf_statement_to_string $statement2]]
  librdf_stream_next $stream
}
librdf_free_stream $stream
librdf_free_statement $statement


# This doesn't seem to work for Tcl

#puts "Writing model to test-out.rdf as rdf/xml";
#
#set serializer [librdf_new_serializer $world NULL NULL NULL]
#if {"$serializer" == "NULL"} then {
#  librdf_free_model $model
#  librdf_free_storage $storage
#  error "failed to create serializer for application/rdfxml"
#}
#
#librdf_serializer_serialize_model_to_file $serializer "test-out.rdf" NULL $model
#librdf_free_serializer $serializer

puts "Done";

librdf_free_model $model
librdf_free_storage $storage

librdf_free_world $world
