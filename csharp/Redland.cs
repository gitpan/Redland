//package Redland;

/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.19
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */


using System;

public class Redland {
  public static SWIGTYPE_p_librdf_world_s librdf_new_world() {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_world();
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_world_s(cPtr, false);
  }

  public static void librdf_free_world(SWIGTYPE_p_librdf_world_s world) {
    RedlandPINVOKE.librdf_free_world(SWIGTYPE_p_librdf_world_s.getCPtr(world));
  }

  public static void librdf_world_open(SWIGTYPE_p_librdf_world_s world) {
    RedlandPINVOKE.librdf_world_open(SWIGTYPE_p_librdf_world_s.getCPtr(world));
  }

  public static void librdf_init_world(string digest_factory_name, SWIGTYPE_p_librdf_hash_s uris_hash) {
    RedlandPINVOKE.librdf_init_world(digest_factory_name, SWIGTYPE_p_librdf_hash_s.getCPtr(uris_hash));
  }

  public static void librdf_destroy_world() {
    RedlandPINVOKE.librdf_destroy_world();
  }

  public static void librdf_free_iterator(SWIGTYPE_p_librdf_iterator_s arg0) {
    RedlandPINVOKE.librdf_free_iterator(SWIGTYPE_p_librdf_iterator_s.getCPtr(arg0));
  }

  public static int librdf_iterator_have_elements(SWIGTYPE_p_librdf_iterator_s iterator) {
    return RedlandPINVOKE.librdf_iterator_have_elements(SWIGTYPE_p_librdf_iterator_s.getCPtr(iterator));
  }

  public static int librdf_iterator_end(SWIGTYPE_p_librdf_iterator_s iterator) {
    return RedlandPINVOKE.librdf_iterator_end(SWIGTYPE_p_librdf_iterator_s.getCPtr(iterator));
  }

  public static SWIGTYPE_p_librdf_node_s librdf_iterator_get_object(SWIGTYPE_p_librdf_iterator_s iterator) {
    IntPtr cPtr = RedlandPINVOKE.librdf_iterator_get_object(SWIGTYPE_p_librdf_iterator_s.getCPtr(iterator));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_iterator_get_context(SWIGTYPE_p_librdf_iterator_s iterator) {
    IntPtr cPtr = RedlandPINVOKE.librdf_iterator_get_context(SWIGTYPE_p_librdf_iterator_s.getCPtr(iterator));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static int librdf_iterator_next(SWIGTYPE_p_librdf_iterator_s iterator) {
    return RedlandPINVOKE.librdf_iterator_next(SWIGTYPE_p_librdf_iterator_s.getCPtr(iterator));
  }

  public static SWIGTYPE_p_librdf_uri_s librdf_new_uri(SWIGTYPE_p_librdf_world_s world, string string) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_uri(SWIGTYPE_p_librdf_world_s.getCPtr(world), string);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_uri_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_uri_s librdf_new_uri_from_uri(SWIGTYPE_p_librdf_uri_s uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_uri_from_uri(SWIGTYPE_p_librdf_uri_s.getCPtr(uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_uri_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_uri_s librdf_new_uri_from_filename(SWIGTYPE_p_librdf_world_s world, string filename) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_uri_from_filename(SWIGTYPE_p_librdf_world_s.getCPtr(world), filename);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_uri_s(cPtr, false);
  }

  public static void librdf_free_uri(SWIGTYPE_p_librdf_uri_s uri) {
    RedlandPINVOKE.librdf_free_uri(SWIGTYPE_p_librdf_uri_s.getCPtr(uri));
  }

  public static string librdf_uri_to_string(SWIGTYPE_p_librdf_uri_s uri) {
    return RedlandPINVOKE.librdf_uri_to_string(SWIGTYPE_p_librdf_uri_s.getCPtr(uri));
  }

  public static int librdf_uri_equals(SWIGTYPE_p_librdf_uri_s first_uri, SWIGTYPE_p_librdf_uri_s second_uri) {
    return RedlandPINVOKE.librdf_uri_equals(SWIGTYPE_p_librdf_uri_s.getCPtr(first_uri), SWIGTYPE_p_librdf_uri_s.getCPtr(second_uri));
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node(SWIGTYPE_p_librdf_world_s world) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node(SWIGTYPE_p_librdf_world_s.getCPtr(world));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_uri_string(SWIGTYPE_p_librdf_world_s world, string string) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_uri_string(SWIGTYPE_p_librdf_world_s.getCPtr(world), string);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_uri(SWIGTYPE_p_librdf_world_s world, SWIGTYPE_p_librdf_uri_s uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_uri(SWIGTYPE_p_librdf_world_s.getCPtr(world), SWIGTYPE_p_librdf_uri_s.getCPtr(uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_literal(SWIGTYPE_p_librdf_world_s world, string string, string xml_language, int is_wf_xml) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_literal(SWIGTYPE_p_librdf_world_s.getCPtr(world), string, xml_language, is_wf_xml);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_typed_literal(SWIGTYPE_p_librdf_world_s world, string string, string xml_language, SWIGTYPE_p_librdf_uri_s datatype_uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_typed_literal(SWIGTYPE_p_librdf_world_s.getCPtr(world), string, xml_language, SWIGTYPE_p_librdf_uri_s.getCPtr(datatype_uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_node(SWIGTYPE_p_librdf_node_s node) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_node(SWIGTYPE_p_librdf_node_s.getCPtr(node));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_new_node_from_blank_identifier(SWIGTYPE_p_librdf_world_s world, string identifier) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_node_from_blank_identifier(SWIGTYPE_p_librdf_world_s.getCPtr(world), identifier);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static void librdf_free_node(SWIGTYPE_p_librdf_node_s r) {
    RedlandPINVOKE.librdf_free_node(SWIGTYPE_p_librdf_node_s.getCPtr(r));
  }

  public static SWIGTYPE_p_librdf_uri_s librdf_node_get_uri(SWIGTYPE_p_librdf_node_s node) {
    IntPtr cPtr = RedlandPINVOKE.librdf_node_get_uri(SWIGTYPE_p_librdf_node_s.getCPtr(node));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_uri_s(cPtr, false);
  }

  public static int librdf_node_get_type(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_type(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static string librdf_node_get_literal_value(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_literal_value(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static string librdf_node_get_literal_value_as_latin1(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_literal_value_as_latin1(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static string librdf_node_get_literal_value_language(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_literal_value_language(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static SWIGTYPE_p_librdf_uri_s librdf_node_get_literal_value_datatype_uri(SWIGTYPE_p_librdf_node_s node) {
    IntPtr cPtr = RedlandPINVOKE.librdf_node_get_literal_value_datatype_uri(SWIGTYPE_p_librdf_node_s.getCPtr(node));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_uri_s(cPtr, false);
  }

  public static int librdf_node_get_literal_value_is_wf_xml(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_literal_value_is_wf_xml(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static string librdf_node_to_string(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_to_string(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static string librdf_node_get_blank_identifier(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_get_blank_identifier(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static int librdf_node_is_resource(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_is_resource(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static int librdf_node_is_literal(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_is_literal(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static int librdf_node_is_blank(SWIGTYPE_p_librdf_node_s node) {
    return RedlandPINVOKE.librdf_node_is_blank(SWIGTYPE_p_librdf_node_s.getCPtr(node));
  }

  public static int librdf_node_equals(SWIGTYPE_p_librdf_node_s first_node, SWIGTYPE_p_librdf_node_s second_node) {
    return RedlandPINVOKE.librdf_node_equals(SWIGTYPE_p_librdf_node_s.getCPtr(first_node), SWIGTYPE_p_librdf_node_s.getCPtr(second_node));
  }

  public static SWIGTYPE_p_librdf_statement_s librdf_new_statement(SWIGTYPE_p_librdf_world_s world) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_statement(SWIGTYPE_p_librdf_world_s.getCPtr(world));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_statement_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_statement_s librdf_new_statement_from_statement(SWIGTYPE_p_librdf_statement_s statement) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_statement_from_statement(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_statement_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_statement_s librdf_new_statement_from_nodes(SWIGTYPE_p_librdf_world_s world, SWIGTYPE_p_librdf_node_s subject, SWIGTYPE_p_librdf_node_s predicate, SWIGTYPE_p_librdf_node_s object) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_statement_from_nodes(SWIGTYPE_p_librdf_world_s.getCPtr(world), SWIGTYPE_p_librdf_node_s.getCPtr(subject), SWIGTYPE_p_librdf_node_s.getCPtr(predicate), SWIGTYPE_p_librdf_node_s.getCPtr(object));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_statement_s(cPtr, false);
  }

  public static void librdf_free_statement(SWIGTYPE_p_librdf_statement_s statement) {
    RedlandPINVOKE.librdf_free_statement(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static SWIGTYPE_p_librdf_node_s librdf_statement_get_subject(SWIGTYPE_p_librdf_statement_s statement) {
    IntPtr cPtr = RedlandPINVOKE.librdf_statement_get_subject(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static void librdf_statement_set_subject(SWIGTYPE_p_librdf_statement_s statement, SWIGTYPE_p_librdf_node_s subject) {
    RedlandPINVOKE.librdf_statement_set_subject(SWIGTYPE_p_librdf_statement_s.getCPtr(statement), SWIGTYPE_p_librdf_node_s.getCPtr(subject));
  }

  public static SWIGTYPE_p_librdf_node_s librdf_statement_get_predicate(SWIGTYPE_p_librdf_statement_s statement) {
    IntPtr cPtr = RedlandPINVOKE.librdf_statement_get_predicate(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static void librdf_statement_set_predicate(SWIGTYPE_p_librdf_statement_s statement, SWIGTYPE_p_librdf_node_s predicate) {
    RedlandPINVOKE.librdf_statement_set_predicate(SWIGTYPE_p_librdf_statement_s.getCPtr(statement), SWIGTYPE_p_librdf_node_s.getCPtr(predicate));
  }

  public static SWIGTYPE_p_librdf_node_s librdf_statement_get_object(SWIGTYPE_p_librdf_statement_s statement) {
    IntPtr cPtr = RedlandPINVOKE.librdf_statement_get_object(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static void librdf_statement_set_object(SWIGTYPE_p_librdf_statement_s statement, SWIGTYPE_p_librdf_node_s object) {
    RedlandPINVOKE.librdf_statement_set_object(SWIGTYPE_p_librdf_statement_s.getCPtr(statement), SWIGTYPE_p_librdf_node_s.getCPtr(object));
  }

  public static string librdf_statement_to_string(SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_statement_to_string(SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static SWIGTYPE_p_librdf_model_s librdf_new_model(SWIGTYPE_p_librdf_world_s world, SWIGTYPE_p_librdf_storage_s storage, string options_string) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_model(SWIGTYPE_p_librdf_world_s.getCPtr(world), SWIGTYPE_p_librdf_storage_s.getCPtr(storage), options_string);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_model_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_model_s librdf_new_model_with_options(SWIGTYPE_p_librdf_world_s world, SWIGTYPE_p_librdf_storage_s storage, SWIGTYPE_p_librdf_hash_s options) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_model_with_options(SWIGTYPE_p_librdf_world_s.getCPtr(world), SWIGTYPE_p_librdf_storage_s.getCPtr(storage), SWIGTYPE_p_librdf_hash_s.getCPtr(options));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_model_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_model_s librdf_new_model_from_model(SWIGTYPE_p_librdf_model_s model) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_model_from_model(SWIGTYPE_p_librdf_model_s.getCPtr(model));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_model_s(cPtr, false);
  }

  public static void librdf_free_model(SWIGTYPE_p_librdf_model_s model) {
    RedlandPINVOKE.librdf_free_model(SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static int librdf_model_size(SWIGTYPE_p_librdf_model_s model) {
    return RedlandPINVOKE.librdf_model_size(SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static int librdf_model_add(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s subject, SWIGTYPE_p_librdf_node_s predicate, SWIGTYPE_p_librdf_node_s object) {
    return RedlandPINVOKE.librdf_model_add(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(subject), SWIGTYPE_p_librdf_node_s.getCPtr(predicate), SWIGTYPE_p_librdf_node_s.getCPtr(object));
  }

  public static int librdf_model_add_typed_literal_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s subject, SWIGTYPE_p_librdf_node_s predicate, string string, string xml_language, SWIGTYPE_p_librdf_uri_s datatype_uri) {
    return RedlandPINVOKE.librdf_model_add_typed_literal_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(subject), SWIGTYPE_p_librdf_node_s.getCPtr(predicate), string, xml_language, SWIGTYPE_p_librdf_uri_s.getCPtr(datatype_uri));
  }

  public static int librdf_model_add_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_model_add_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static int librdf_model_add_statements(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_stream_s statement_stream) {
    return RedlandPINVOKE.librdf_model_add_statements(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_stream_s.getCPtr(statement_stream));
  }

  public static int librdf_model_remove_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_model_remove_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static int librdf_model_contains_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_model_contains_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_model_as_stream(SWIGTYPE_p_librdf_model_s model) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_as_stream(SWIGTYPE_p_librdf_model_s.getCPtr(model));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_model_serialise(SWIGTYPE_p_librdf_model_s model) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_serialise(SWIGTYPE_p_librdf_model_s.getCPtr(model));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_model_find_statements(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_statement_s statement) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_find_statements(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_iterator_s librdf_model_get_sources(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s arc, SWIGTYPE_p_librdf_node_s target) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_sources(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(arc), SWIGTYPE_p_librdf_node_s.getCPtr(target));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_iterator_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_iterator_s librdf_model_get_arcs(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s source, SWIGTYPE_p_librdf_node_s target) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_arcs(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(source), SWIGTYPE_p_librdf_node_s.getCPtr(target));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_iterator_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_iterator_s librdf_model_get_targets(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s source, SWIGTYPE_p_librdf_node_s arc) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_targets(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(source), SWIGTYPE_p_librdf_node_s.getCPtr(arc));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_iterator_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_model_get_source(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s arc, SWIGTYPE_p_librdf_node_s target) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_source(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(arc), SWIGTYPE_p_librdf_node_s.getCPtr(target));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_model_get_arc(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s source, SWIGTYPE_p_librdf_node_s target) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_arc(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(source), SWIGTYPE_p_librdf_node_s.getCPtr(target));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_model_get_target(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s source, SWIGTYPE_p_librdf_node_s arc) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_get_target(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(source), SWIGTYPE_p_librdf_node_s.getCPtr(arc));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static int librdf_model_context_add_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context, SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_model_context_add_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static int librdf_model_context_add_statements(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context, SWIGTYPE_p_librdf_stream_s stream) {
    return RedlandPINVOKE.librdf_model_context_add_statements(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context), SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
  }

  public static int librdf_model_context_remove_statement(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context, SWIGTYPE_p_librdf_statement_s statement) {
    return RedlandPINVOKE.librdf_model_context_remove_statement(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context), SWIGTYPE_p_librdf_statement_s.getCPtr(statement));
  }

  public static int librdf_model_context_remove_statements(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context) {
    return RedlandPINVOKE.librdf_model_context_remove_statements(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context));
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_model_context_as_stream(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_context_as_stream(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_model_context_serialize(SWIGTYPE_p_librdf_model_s model, SWIGTYPE_p_librdf_node_s context) {
    IntPtr cPtr = RedlandPINVOKE.librdf_model_context_serialize(SWIGTYPE_p_librdf_model_s.getCPtr(model), SWIGTYPE_p_librdf_node_s.getCPtr(context));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static void librdf_model_sync(SWIGTYPE_p_librdf_model_s model) {
    RedlandPINVOKE.librdf_model_sync(SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static SWIGTYPE_p_librdf_storage_s librdf_new_storage(SWIGTYPE_p_librdf_world_s world, string storage_name, string name, string options_string) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_storage(SWIGTYPE_p_librdf_world_s.getCPtr(world), storage_name, name, options_string);
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_storage_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_storage_s librdf_new_storage_from_storage(SWIGTYPE_p_librdf_storage_s old_storage) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_storage_from_storage(SWIGTYPE_p_librdf_storage_s.getCPtr(old_storage));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_storage_s(cPtr, false);
  }

  public static void librdf_free_storage(SWIGTYPE_p_librdf_storage_s storage) {
    RedlandPINVOKE.librdf_free_storage(SWIGTYPE_p_librdf_storage_s.getCPtr(storage));
  }

  public static SWIGTYPE_p_librdf_parser_s librdf_new_parser(SWIGTYPE_p_librdf_world_s world, string name, string mime_type, SWIGTYPE_p_librdf_uri_s type_uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_parser(SWIGTYPE_p_librdf_world_s.getCPtr(world), name, mime_type, SWIGTYPE_p_librdf_uri_s.getCPtr(type_uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_parser_s(cPtr, false);
  }

  public static void librdf_free_parser(SWIGTYPE_p_librdf_parser_s parser) {
    RedlandPINVOKE.librdf_free_parser(SWIGTYPE_p_librdf_parser_s.getCPtr(parser));
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_parser_parse_as_stream(SWIGTYPE_p_librdf_parser_s parser, SWIGTYPE_p_librdf_uri_s uri, SWIGTYPE_p_librdf_uri_s base_uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_parser_parse_as_stream(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), SWIGTYPE_p_librdf_uri_s.getCPtr(uri), SWIGTYPE_p_librdf_uri_s.getCPtr(base_uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static int librdf_parser_parse_into_model(SWIGTYPE_p_librdf_parser_s parser, SWIGTYPE_p_librdf_uri_s uri, SWIGTYPE_p_librdf_uri_s base_uri, SWIGTYPE_p_librdf_model_s model) {
    return RedlandPINVOKE.librdf_parser_parse_into_model(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), SWIGTYPE_p_librdf_uri_s.getCPtr(uri), SWIGTYPE_p_librdf_uri_s.getCPtr(base_uri), SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static SWIGTYPE_p_librdf_stream_s librdf_parser_parse_string_as_stream(SWIGTYPE_p_librdf_parser_s parser, string string, SWIGTYPE_p_librdf_uri_s base_uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_parser_parse_string_as_stream(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), string, SWIGTYPE_p_librdf_uri_s.getCPtr(base_uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_stream_s(cPtr, false);
  }

  public static int librdf_parser_parse_string_into_model(SWIGTYPE_p_librdf_parser_s parser, string string, SWIGTYPE_p_librdf_uri_s base_uri, SWIGTYPE_p_librdf_model_s model) {
    return RedlandPINVOKE.librdf_parser_parse_string_into_model(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), string, SWIGTYPE_p_librdf_uri_s.getCPtr(base_uri), SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static string librdf_parser_get_feature(SWIGTYPE_p_librdf_parser_s parser, SWIGTYPE_p_librdf_uri_s feature) {
    return RedlandPINVOKE.librdf_parser_get_feature(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), SWIGTYPE_p_librdf_uri_s.getCPtr(feature));
  }

  public static int librdf_parser_set_feature(SWIGTYPE_p_librdf_parser_s parser, SWIGTYPE_p_librdf_uri_s feature, string value) {
    return RedlandPINVOKE.librdf_parser_set_feature(SWIGTYPE_p_librdf_parser_s.getCPtr(parser), SWIGTYPE_p_librdf_uri_s.getCPtr(feature), value);
  }

  public static SWIGTYPE_p_librdf_serializer librdf_new_serializer(SWIGTYPE_p_librdf_world_s world, string name, string mime_type, SWIGTYPE_p_librdf_uri_s type_uri) {
    IntPtr cPtr = RedlandPINVOKE.librdf_new_serializer(SWIGTYPE_p_librdf_world_s.getCPtr(world), name, mime_type, SWIGTYPE_p_librdf_uri_s.getCPtr(type_uri));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_serializer(cPtr, false);
  }

  public static void librdf_free_serializer(SWIGTYPE_p_librdf_serializer serializer) {
    RedlandPINVOKE.librdf_free_serializer(SWIGTYPE_p_librdf_serializer.getCPtr(serializer));
  }

  public static int librdf_serializer_serialize_model_to_file(SWIGTYPE_p_librdf_serializer serializer, string name, SWIGTYPE_p_librdf_uri_s base_uri, SWIGTYPE_p_librdf_model_s model) {
    return RedlandPINVOKE.librdf_serializer_serialize_model_to_file(SWIGTYPE_p_librdf_serializer.getCPtr(serializer), name, SWIGTYPE_p_librdf_uri_s.getCPtr(base_uri), SWIGTYPE_p_librdf_model_s.getCPtr(model));
  }

  public static string librdf_serializer_get_feature(SWIGTYPE_p_librdf_serializer serializer, SWIGTYPE_p_librdf_uri_s feature) {
    return RedlandPINVOKE.librdf_serializer_get_feature(SWIGTYPE_p_librdf_serializer.getCPtr(serializer), SWIGTYPE_p_librdf_uri_s.getCPtr(feature));
  }

  public static int librdf_serializer_set_feature(SWIGTYPE_p_librdf_serializer serializer, SWIGTYPE_p_librdf_uri_s feature, string value) {
    return RedlandPINVOKE.librdf_serializer_set_feature(SWIGTYPE_p_librdf_serializer.getCPtr(serializer), SWIGTYPE_p_librdf_uri_s.getCPtr(feature), value);
  }

  public static void librdf_free_stream(SWIGTYPE_p_librdf_stream_s stream) {
    RedlandPINVOKE.librdf_free_stream(SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
  }

  public static int librdf_stream_end(SWIGTYPE_p_librdf_stream_s stream) {
    return RedlandPINVOKE.librdf_stream_end(SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
  }

  public static int librdf_stream_next(SWIGTYPE_p_librdf_stream_s stream) {
    return RedlandPINVOKE.librdf_stream_next(SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
  }

  public static SWIGTYPE_p_librdf_statement_s librdf_stream_get_object(SWIGTYPE_p_librdf_stream_s stream) {
    IntPtr cPtr = RedlandPINVOKE.librdf_stream_get_object(SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_statement_s(cPtr, false);
  }

  public static SWIGTYPE_p_librdf_node_s librdf_stream_get_context(SWIGTYPE_p_librdf_stream_s stream) {
    IntPtr cPtr = RedlandPINVOKE.librdf_stream_get_context(SWIGTYPE_p_librdf_stream_s.getCPtr(stream));
    return (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_librdf_node_s(cPtr, false);
  }

  public static void librdf_internal_test_error(SWIGTYPE_p_librdf_world_s world) {
    RedlandPINVOKE.librdf_internal_test_error(SWIGTYPE_p_librdf_world_s.getCPtr(world));
  }

  public static void librdf_internal_test_warning(SWIGTYPE_p_librdf_world_s world) {
    RedlandPINVOKE.librdf_internal_test_warning(SWIGTYPE_p_librdf_world_s.getCPtr(world));
  }

  public static string redland_copyright_string {
    get {
      return RedlandPINVOKE.get_redland_copyright_string();
    } 
  }

  public static string redland_version_string {
    get {
      return RedlandPINVOKE.get_redland_version_string();
    } 
  }

  public static int redland_version_major {
    get {
      return RedlandPINVOKE.get_redland_version_major();
    } 
  }

  public static int redland_version_minor {
    get {
      return RedlandPINVOKE.get_redland_version_minor();
    } 
  }

  public static int redland_version_release {
    get {
      return RedlandPINVOKE.get_redland_version_release();
    } 
  }

}
