# -*- Mode: Perl -*-
#
# Node.pm - Redland Perl RDF Node module
#
# $Id: Node.pm,v 1.34 2003/08/28 19:49:56 cmdjb Exp $
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

package RDF::Redland::Node;

use strict;

use vars qw($Type_Resource $Type_Property $Type_Literal
	    $Type_Statement $Type_Blank);

# FIXME: Should be the same as values of librdf_node_type enum in rdf_node.h
# and mechanically kept in sync.
$Type_Resource  = 1;
$Type_Literal   = 2;
$Type_Blank     = 4;
# FIXME: Needs to also match documentation near sub type



=pod

=head1 NAME

RDF::Redland::Node - Redland RDF Node (RDF Resource, Property, Literal) Class

=head1 SYNOPSIS

  use RDF::Redland;
  my $node1=new RDF::Redland::Node("Hello, World!");
  my $node2=new RDF::Redland::Node($uri);
  my $node3=$node2->clone;

  my $node4=RDF::Redland::Node->new_from_uri("http://example.com/");
  my $node5=RDF::Redland::Node->new_literal("Hello, World!");
  my $node6=RDF::Redland::Node->new_xml_literal("<tag>content</tag>");
  my $node7=RDF::Redland::Node->new_from_blank_identifier("genid1");
  ...

  print $node4->uri->as_string,"\n";  # Using RDF::Redland::URI::as_string
  print $node5->literal_value_as_latin1,"\n";

=head1 DESCRIPTION

This class represents RDF URIs, literals and blank nodes in the RDF graph.  

=cut

######################################################################

=pod

=head1 CONSTRUCTORS

=over

=item new [STRING | URI | NODE]

Create a new URI node, literal node or copy an existing node.

If a literal I<STRING> is given, make a plain literal node.  If a
I<URI> class (perl URI or RDF::Redland::URI), make a resource node.
Otherwise if an existing (RDF::Redland::Node) I<NODE> is given, copy
the existing node.

=cut


# CONSTRUCTOR
# (main)
sub new ($;$) {
  my($proto,$arg)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};

  if($arg) {
    if(my $arg_class=ref($arg)) {
      # Try several classes
      if(UNIVERSAL::isa($arg, 'RDF::Redland::Node')) {
        return $arg->clone;
      } elsif(UNIVERSAL::isa($arg, 'RDF::Redland::URI')) {
        $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$arg->as_string);
      } elsif (UNIVERSAL::isa($arg, 'URI::URL')) {
	$self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$arg->as_string);
      } else {
	die "RDF::Redland::Node::new - Cannot make a node from an object of class $arg_class\n";
      }
    } else {
      # Not a class
      $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_typed_literal($RDF::Redland::World->{WORLD},$arg,'',undef);
    }
  } else {
    $self->{NODE}=&RDF::Redland::CORE::librdf_new_node($RDF::Redland::World->{WORLD});
  }
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}

sub new_from_uri_string ($$) {
  my($proto,$uri_string)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};
  die "RDF::Redland::Node::new_from_uri_string - Cannot create node from empty URI\n"
    unless $uri_string;

  $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$uri_string);
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}

=item new_from_uri URI

Create a new URI node.  I<URI> can be either a RDF::Redland::URI
object, a perl URI class or a literal string.

=cut

sub new_from_uri ($$) {
  my($proto,$arg)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};
  if(my $class=ref $arg) {
    if(UNIVERSAL::isa($arg, 'RDF::Redland::URI')) {
      $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri($RDF::Redland::World->{WORLD},$arg->{URI});
    } elsif (UNIVERSAL::isa($arg, 'URI::URL')) {
      $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$arg->as_string);
    } else {
      die "RDF::Redland::Node::new_from_uri - Cannot make a Node from an object of class $class\n";
    }
  } else {
    $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$arg);
  }
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}


sub new_from_literal ($$$$) {
  my($proto,$string,$xml_language,$is_wf_xml)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};
  $is_wf_xml=($is_wf_xml ? 1 : 0);
  $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_literal($RDF::Redland::World->{WORLD},$string,$xml_language,$is_wf_xml);
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}

=item new_literal STRING [DATATYPE [XML_LANGUAGE]]

Create a new literal node for a literal value I<STRING>.
Optional datatype URI I<DATATYPE> (RDF::Redland::URI, perl URI or string)
and language (xml:lang attribute) I<XML_LANGUAGE> may also be given.

=cut

sub new_literal ($$;$$) {
  my($proto,$string,$dt,$xml_language)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};
  my $dt_uri=undef;
  if(defined $dt) {
    if(UNIVERSAL::isa($dt, 'RDF::Redland::URI')) {
      $dt_uri=$dt;
    } elsif (UNIVERSAL::isa($dt, 'URI::URL')) {
      $dt_uri=new RDF::Redland::URI($dt->as_string);
    } else {
      $dt_uri=new RDF::Redland::URI($dt);
    }
  }

  $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_typed_literal($RDF::Redland::World->{WORLD},$string,$xml_language,$dt_uri);
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}

=item new_xml_literal STRING

Create a new XML datatyped literal node for the XML in I<STRING>.

=cut

sub new_xml_literal ($$) {
  my($proto,$string)=@_;
  return $proto->new_from_literal($string,'',1);
}


=item new_from_blank_identifier IDENTIFIER

Create a new blank node with blank node identifier I<IDENTIFIER>.

=cut

sub new_from_blank_identifier ($;$) {
  my($proto,$identifier)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};
  $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_blank_identifier($RDF::Redland::World->{WORLD},$identifier);
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}


=item clone

Copy a RDF::Redland::Node.

=cut

sub clone ($) {
  my($node)=@_;
  my $class = ref($node);
  my $self  = {};

  if(!$class || $class ne 'RDF::Redland::Node') {
    die "RDF::Redland::Node::clone - Cannot copy a node object not of class RDF::Redland::Node\n";
  }

  $self->{NODE}=&RDF::Redland::CORE::librdf_new_node_from_node($node->{NODE});
  return undef if !$self->{NODE};

  bless ($self, $class);
  return $self;
}

sub new_from_node ($$) {
  my($proto,$node)=@_;
  return $node->clone;
}

# internal constructor to build an object from a node created
# by librdf e.g. from the result of a iterator->next operation
# It always makes a new Redland Node
sub _new_from_object ($$;$) {
  my($proto,$object,$do_not_copy)=@_;
  return undef if !$object;
  my $class = ref($proto) || $proto;
  my $self  = {};

  $self->{NODE}=$do_not_copy ? $object : &RDF::Redland::CORE::librdf_new_node_from_node($object);
  bless ($self, $class);
  return $self;
}

=pod

=back

=cut

# DESTRUCTOR
sub DESTROY ($) {
  my $self=shift;
  warn "RDF::Redland::Node DESTROY $self" if $RDF::Redland::Debug;
  if($self->{NODE}) {
    &RDF::Redland::CORE::librdf_free_node($self->{NODE});
  }
  warn "RDF::Redland::Node DESTROY done\n" if $RDF::Redland::Debug;
}


=head1 METHODS

=over

=item uri

Get the current URI of the node as an RDF::Redland::URI object.

=cut

sub uri ($) {
  my $obj=&RDF::Redland::CORE::librdf_node_get_uri(shift->{NODE});
  return $obj ?  RDF::Redland::URI->_new_from_object($obj) : undef;
}

=item blank_identifier

Get the current blank identifier of the node

=cut

sub blank_identifier ($) {
  return &RDF::Redland::CORE::librdf_node_get_blank_identifier(shift->{NODE});
}

=item type

Get the node type.  It is recommended to use the is_resource, is_literal
or is_blank methods in preference to this (both simpler and quicker).

The current list of types that are supported are:

  $RDF::Redland::Node::Type_Resource
  $RDF::Redland::Node::Type_Literal
  $RDF::Redland::Node::Type_Blank

Example:

  if ($node->type == $RDF::Redland::Node::Type_Resource) {
    print "Node is a resource with URI ", $node->uri->as_string, "\n";
  } else {
    ...
  }

=cut

sub type ($) {
  return &RDF::Redland::CORE::librdf_node_get_type(shift->{NODE});
}

=item is_resource

Return true if node is a resource (with a URI)

=cut
sub is_resource($) {
  return &RDF::Redland::CORE::librdf_node_is_resource(shift->{NODE});
}

=item is_literal

Return true if node is a literal

=cut
sub is_literal($) {
  return &RDF::Redland::CORE::librdf_node_is_literal(shift->{NODE});
}

=item is_blank

Return true if node is a blank nodeID

=cut
sub is_blank($) {
  return &RDF::Redland::CORE::librdf_node_is_blank(shift->{NODE});
}

=item literal_value

Get the node literal value string as UTF-8 (when the node is of type
$RDF::Redland::Node::Type_Literal)

=cut

sub literal_value ($) {
  &RDF::Redland::CORE::librdf_node_get_literal_value(shift->{NODE});
}

=item literal_value_as_latin1

Get the node literal value string converted from UTF-8 to ISO Latin-1
(when the node is of type $RDF::Redland::Node::Type_Literal)

=cut

sub literal_value_as_latin1 ($) {
  &RDF::Redland::CORE::librdf_node_get_literal_value_as_latin1(shift->{NODE});
}

=item literal_value_language

Get the node literal XML language (when the node is of type
$RDF::Redland::Node::Type_Literal) or undef if not present.

=cut

sub literal_value_language ($) {
  &RDF::Redland::CORE::librdf_node_get_literal_value_language(shift->{NODE});
}

=item literal_value_is_wf_xml

Return non 0 if the literal string is well formed XML (when the node
is of type $RDF::Redland::Node::Type_Literal).

=cut

sub literal_value_is_wf_xml ($) {
  &RDF::Redland::CORE::librdf_node_get_literal_value_is_wf_xml(shift->{NODE});
}

=item literal_datatype

Return the RDF::Redland::URI of the literal datatype or undef if it
is not a datatype.

=cut

sub literal_datatype($) {
  my $self=shift;
  my $uri=&RDF::Redland::CORE::librdf_node_get_literal_value_datatype_uri($self->{NODE});
  return $uri ? RDF::Redland::URI->new(&RDF::Redland::CORE::librdf_uri_to_string($uri)) : undef;
}

=item as_string

Return the RDF::Redland::Node formatted as a string (UTF-8 encoded).

=cut

sub as_string ($) {
  &RDF::Redland::CORE::librdf_node_to_string(shift->{NODE});
}

=item equals NODE

Return non zero if this node is equal to NODE

=cut

sub equals ($$) {
  my($self,$node)=@_;
  &RDF::Redland::CORE::librdf_node_equals($self->{NODE}, $node->{NODE});
}


# Ensure the thing given is a Redland node, promoting it if possible
# from other perl objects.
sub _ensure ($) {
  my $node=shift;
  if(UNIVERSAL::isa($node, 'RDF::Redland::Node')) {
    $node=&RDF::Redland::CORE::librdf_new_node_from_node($node->{NODE});
  } elsif(UNIVERSAL::isa($node, 'RDF::Redland::URI')) {
    $node=&RDF::Redland::CORE::librdf_new_node_from_uri($RDF::Redland::World->{WORLD},$node->{URI});
  } elsif (UNIVERSAL::isa($node, 'URI::URL')) {
    $node=&RDF::Redland::CORE::librdf_new_node_from_uri_string($RDF::Redland::World->{WORLD},$node->as_string);	
  } else {
    $node=undef;
  }
  return $node;
}


=pod

=back

=head1 OLDER METHODS

=over

=item new_from_literal STRING XML_LANGUAGE IS_WF

Create a new RDF::Redland::Node object for a literal value I<STRING> with XML
language (xml:lang attribute) I<XML_LANGUAGE>
and if content is well formed XML, when I<IS_WF> is non
0.  I<XML_LANGUAGE> is optional can can be set to undef.

This method remains but using new_literal is prefered.
For plain literals $node=new RDF::Redland::Node("blah") is simplest.

=item new_from_typed_literal STRING [DATATYPE [XML_LANGUAGE]]

Renamed to new_literal with same arguments.

=item new_from_uri_string URI_STRING

Create a new RDF::Redland::Node object for a resource with URI I<URI_STRING>.
It is equivalent to use the shorter
$a=new RDF::Redland::Node->new_from_uri($uri_string)

=item new_from_node NODE

Create a new RDF::Redland::Node object from existing
RDF::Redland::Node I<NODE> (copy constructor).
It is equivalent to use $new_node=$old_node->clone

=back


=head1 SEE ALSO

L<RDF::Redland::Statement>

=head1 AUTHOR

Dave Beckett - http://purl.org/net/dajobe/

=cut

1;