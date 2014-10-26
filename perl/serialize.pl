#!/usr/bin/perl -w
#
# serialize.pl - Redland serialize test program
#
# $Id: serialize.pl 10592 2006-03-05 08:28:27Z cmdjb $
#
# Copyright (C) 2002-2004 David Beckett - http://purl.org/net/dajobe/
# Copyright (C) 2002-2004 University of Bristol - http://www.bristol.ac.uk/
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
# Status: incomplete
#
#
#

use strict;

use RDF::Redland;

die <<"EOT" if @ARGV < 1 || @ARGV > 2;
Usage: $0 <URI> [BASE URI>]

This program serializes the file URI to RDF/XML using optional
base URI BASE URI if given.

EOT

my $uri=$ARGV[0];

my $tmp_file;
my $source_uri;
if($uri !~ m%^file:%) {
  eval 'use URI::URL';
  eval 'use LWP::Simple';
  
  $tmp_file="/tmp/$0-$$.serialize";

  my $perl_uri;
  eval "\$perl_uri=new URI::URL('$uri')";
  if($@) {
    die "$0: URI $uri is not supported by Perl\n";
  }
  my $rc=getstore($perl_uri, $tmp_file);
  
  if(!is_success($rc)) {
    die "$0: Failed to fetch URI $uri - HTTP error $rc\n";
    unlink $tmp_file;
  }
  $source_uri="file:$tmp_file";
} else {
  $source_uri=$uri;
}
my $base_uri=$uri;
$base_uri=$ARGV[1] if @ARGV ==2;


my $storage=new RDF::Redland::Storage("hashes", "serialize", 
				      "new='yes',hash-type='bdb',dir='.'");
die "$0: Failed to create RDF::Redland::Storage\n" unless $storage;

my $model=new RDF::Redland::Model($storage, "");
die "$0: Failed to create RDF::Redland::Model for storage\n" unless $model;

my $parser=new RDF::Redland::Parser("rdfxml");
die "$0: Failed to create RDF::Redland::Parser 'rdfxml'\n" unless $parser;

my $redland_base_uri=new RDF::Redland::URI $uri;
my $redland_source_uri=new RDF::Redland::URI $source_uri;

my $stream=$parser->parse_as_stream($redland_source_uri, $redland_base_uri);
die "$0: Failed to create stream using parse_as_stream\n"
  if !$stream || $stream->end;
my $count=0;
while(!$stream->end) {
  $model->add_statement($stream->current);
  $count++;
  $stream->next;
}
$stream=undef;
print "URI \"$uri\" parsed as RDF/XML OK (created $count statements)\n";

unlink $tmp_file if $tmp_file;

## SERIALIZE test

my $RDF_NS="http://www.w3.org/1999/02/22-rdf-syntax-ns#";
my $XML_NS="http://www.w3.org/XML/1998/namespace";
my $RDF_type_predicate=RDF::Redland::Node->new_from_uri($RDF_NS."type");

my $DC_NS="http://purl.org/dc/elements/1.1";

my $state={
  inscope_namespaces => [
    ["xml" , $XML_NS, -1],
  ],
  favourite_prefixes => {
    $RDF_NS => "rdf",
    $DC_NS  => "dc",
  },
  ns_count => 0,  # for generating ns0:, ns1: etc.
  feature_prefer_ID_to_about => "false", # prefer rdf:ID or rdf:about ?
  feature_indenting_size => 2,
  feature_content_encoding => "utf8",
  feature_prefer_default_ns_rdf => "false",
  depth => 0,                            # depth of output XML elements

  node_types => {},
  nodes => {},
  subject_key_to_node => {},
};


sub predicate_split($) {
  my $predicate=shift;
  my $uri=$predicate->uri->as_string;
  if($uri =~ m%^(.*[#/])(.*)$%) {
    return($1,$2);
  }
  return($uri,undef);
}


$stream=$model->as_stream;
my $id=1;
while(!$stream->end) {
  my $statement=$stream->current;
  last if !$statement;
  my $subject=$statement->subject;
  my $subject_key=$subject->as_string;

  my($max_uri,$name)=predicate_split($statement->predicate);
  if(!$name) {
    warn "$0: Statenent with predicate URI $max_uri cannot be serialized\n";
    next;
  }

  if($statement->predicate->equals($RDF_type_predicate)) {
    push(@{$state->{node_types}->{$subject_key}}, $statement);
  } else {
    push(@{$state->{nodes}->{$subject_key}}, $statement);
  }
  $state->{subject_key_to_node}->{$subject_key}=$subject;
  $stream->next;
}


sub make_xml_qname($$$;$) {
  my($state,$ns_uri,$name,$is_attr)=@_;
  for my $nspace (@{$state->{inscope_namespaces}}) {
    my($prefix,$nspace_uri)=@$nspace;
    next if $nspace_uri ne $ns_uri;
    next if ($is_attr && !length $prefix);

    return $prefix ? "$prefix:$name" : $name;
  }
  die "Namespace URI $ns_uri with a prefix is not declared in current scope\n"
    if $is_attr;
  die "Namespace URI $ns_uri is not declared in current scope\n";
}


sub declare_namespace($$;$) {
  my($state,$ns_uri,$prefix)=@_;
  if(!defined $prefix) {
    $prefix=$state->{favourite_prefixes}->{$ns_uri};
    die "No prefix given for Namespace URI $ns_uri and no favourite found\n"
      unless $prefix;
  }
  unshift(@{$state->{inscope_namespaces}},
	  [$prefix, $ns_uri, $state->{depth}]);
}


sub ensure_declared_namespace($$) {
  my($state,$ns_uri)=@_;
  my $ns_prefix=undef;
  for my $ns (@{$state->{inscope_namespaces}}) {
    my($prefix,$uri,$depth)=@$ns;
    if($uri eq $ns_uri) {
      $ns_prefix=$prefix;
      last;
    }
  }
  return $ns_prefix if $ns_prefix;
  $ns_prefix=$state->{favourite_prefixes}->{$ns_uri};
  if(!$ns_prefix) {
    $ns_prefix="ns" . $state->{ns_count}++;
  }
  declare_namespace($state, $ns_uri, $ns_prefix);
}


sub undeclare_namespaces($) {
  my $state=shift;
  my(@nn);
  for my $nspace (@{$state->{inscope_namespaces}}) {
    push(@nn, $nspace) if $nspace->[2] >= $state->{depth};
  }
  $state->{inscope_namespaces}=\@nn;
}


sub indent($) {
  my $state=shift;
  $state->{depth}++;
}


sub outdent($) {
  my $state=shift;
  $state->{depth}--;
}


sub emit($$) {
  my($state,$string)=@_;

  my $unit=$state->{feature_indenting_size};
  print ' ' x ($state->{depth} * $unit);
  print $string;
}

sub format_element_content ($) {
  my $string=shift;
  $string =~ s/\&/\&amp;/g;
  $string =~ s/</\&lt;/g;
  $string =~ s/>/\&gt;/g;
  $string;
}

sub format_attribute ($) {
  my $string=shift;
  $string = format_element_content($string);
  $string =~ s/"/\&quot;/g; #"
  $string;
}

sub emit_start_element($$@) {
  my($state,$name,@attrs)=@_;

  my $str="<".$name;

  if(my $namespaces=$state->{inscope_namespaces}) {
    for my $ns (@$namespaces) {
      my($prefix, $uri, $depth)=@$ns;
      $str.=qq{ xmlns:$prefix="$uri"}
        if $depth == $state->{depth};
    }
  }

  if(my $lang=$state->{xml_lang}) {
    $str.=qq{ xml:lang="$lang"};
  }

  if(my $base=$state->{xml_base}) {
    $str.=qq{ xml:base="$base"};
  }

  for my $attr (@attrs) {
    my($name,$value)=@$attr;
    $str.=qq{ $name="}.format_attribute($value).qq{"};
  }

  $str.=">";
  emit($state, $str);
}

sub emit_empty_element($$@) {
  my($state,$name,@attrs)=@_;

  my $str="<".$name;

  if(my $namespaces=$state->{inscope_namespaces}) {
    for my $ns (@$namespaces) {
      my($prefix, $uri, $depth)=@$ns;
      $str.=qq{ xmlns:$prefix="$uri"}
        if $depth == $state->{depth};
    }
  }

  if(my $lang=$state->{xml_lang}) {
    $str.=qq{ xml:lang="$lang"};
  }

  if(my $base=$state->{xml_base}) {
    $str.=qq{ xml:base="$base"};
  }

  for my $attr (@attrs) {
    my($name,$value)=@$attr;
    $str.=qq{ $name="}.format_attribute($value).qq{"};
  }

  $str.=" />\n";
  emit($state, $str);
}

sub emit_literal_element($$$@) {
  my($state,$name,$literal,@attrs)=@_;

  my $str="<".$name;

  if(my $namespaces=$state->{inscope_namespaces}) {
    for my $ns (@$namespaces) {
      my($prefix, $uri, $depth)=@$ns;
      $str.=qq{ xmlns:$prefix="$uri"}
        if $depth == $state->{depth};
    }
  }

  if(my $lang=$state->{xml_lang}) {
    $str.=qq{ xml:lang="$lang"};
  }

  if(my $base=$state->{xml_base}) {
    $str.=qq{ xml:base="$base"};
  }

  for my $attr (@attrs) {
    my($name,$value)=@$attr;
    $str.=qq{ $name="}.format_attribute($value).qq{"};
  }

  $str.=">".format_element_content($literal)."</$name>\n";
  emit($state, $str);
}

sub emit_end_element($$) {
  my($state, $name)=@_;
  emit($state, qq{</$name>\n});
}


# print <predicate>object </predicate>
# in various forms depending on type of object node
sub format_statement ($$$) {
  my($state,$predicate,$object)=@_;

  my($predicate_uri,$predicate_name)=predicate_split($predicate);
  
  ensure_declared_namespace($state, $predicate_uri);

  my $qname=make_xml_qname($state, $predicate_uri, $predicate_name);

  if($object->is_resource) {
    my $attr=make_xml_qname($state, $RDF_NS, "resource", 1);
    my $object_value=$object->uri->as_string;
    emit_empty_element($state, $qname, [$attr, $object_value]);
  } elsif($object->is_literal) {
    my $literal=$object->literal_value; # FIXME or literal_value_as_latin1
    my $literal_lang=$object->literal_value_language;
    my $dt_uri=$object->literal_datatype;
    my(@attrs);
    if($literal_lang) {
      my $attr=make_xml_qname($state, $XML_NS, "lang", 1);
      push(@attrs, [$attr, $literal_lang]);
    }
    if ($object->literal_value_is_wf_xml) {
      my $attr=make_xml_qname($state, $RDF_NS, "parseType", 1);
      push(@attrs, [$attr, "Literal"]);
    } elsif ($dt_uri) {
      my $attr=make_xml_qname($state, $RDF_NS, "datatype", 1);
      push(@attrs, [$attr, $dt_uri->as_string]);
    }
    emit_literal_element($state, $qname, $literal, @attrs);
  } elsif($object->is_blank) {
    my $attr=make_xml_qname($state, $RDF_NS, "nodeID");
    my $object_value=$object->blank_identifier;
    emit_empty_element($state, $qname, [$attr, $object_value]);
  } else {
    my $otype=$object->type;
    die "Unknown object type $otype\n";
  }
}


sub start_format_subject($$) {
  my($state,$subject_node)=@_;

  my $element=make_xml_qname($state, $RDF_NS, "Description");
  my($attr, $value);
  if($subject_node->is_resource) {
    $attr=make_xml_qname($state, $RDF_NS, "about", 1);
    $value=$subject_node->uri->as_string;
  } elsif($subject_node->is_blank) {
    $attr=make_xml_qname($state, $RDF_NS, "nodeID", 1);
    $value=$subject_node->blank_identifier;
  } else {
    my $stype=$subject_node->type;
    die "Unknown subject type $stype\n";
  }
  emit_start_element($state, $element, [$attr, $value]);
  print "\n";
}


sub end_format_subject($$) {
  my($state,$subject_node)=@_;

  my $element=make_xml_qname($state, $RDF_NS, "Description");
  emit_end_element($state, $element);
  print "\n";
}


sub dump_nodes ($@) {
  my($state,@order)=@_;

  for my $subject_key (@order) {
    my $subject_node=$state->{subject_key_to_node}->{$subject_key};
    # print "<!-- key $subject_key -->\n";
    
    start_format_subject($state, $subject_node);

    indent($state);
    for my $type (@{$state->{node_types}->{$subject_key}}) {
      format_statement($state, $RDF_type_predicate, $type);
    }
    for my $statement (@{$state->{nodes}->{$subject_key}}) {
      format_statement($state, $statement->predicate, $statement->object);
    }
    outdent($state);

    end_format_subject($state, $subject_node);
  }

}


# start

#use Data::Dumper;
#print Dumper($state),"\n";

my(@nodes_with_types)=keys %{$state->{node_types}};
my(@other_nodes)=grep(!exists $state->{node_types}->{$_}, 
		      keys %{$state->{nodes}});

# order:
#   rdf:ID things first
#   rdf:about things with types?
#   rest

my(@order)=(@nodes_with_types, @other_nodes);

print qq{<?xml version='1.0' encoding='}; #'
print $state->{feature_content_encoding}, "'?>\n"; #'

my $OWL_NS='http://owl.mindswap.org/2003/ont/owlweb.rdf#';
$state->{favourite_prefixes}->{$OWL_NS}='owl';
ensure_declared_namespace($state, $OWL_NS);

ensure_declared_namespace($state, $RDF_NS);
my $element=make_xml_qname($state, $RDF_NS, "RDF");

emit_start_element($state, $element);
print "\n";
  indent($state);
  dump_nodes($state, @order);
  outdent($state);

emit_end_element($state, $element);
print "\n";

exit 0;


package Element;

sub new ($$$) {
  my($proto,$ns_uri,$local_name)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {
    namespace_URI => $ns_uri,
    name => $local_name,
    prefix => undef,
    children => undef,
    parent => undef,
    attributes => undef,
    xml_namespaces => [], # lists of [URI,prefix]
    xml_lang => undef,
    xml_base => undef,
  };

  bless ($self, $class);
  return $self;
}


sub uri ($;$) {
  my($self,$uri)=@_;
  my $old=$self->{namespace_URI};
  $self->{namespace_URI}=$uri if $uri;
  $old;
}


sub prefix ($;$) {
  my($self,$prefix)=@_;
  my $old=$self->{prefix};
  $self->{prefix}=$prefix if $prefix;
  $old;
}


sub lang ($;$) {
  my($self,$lang)=@_;
  my $old=$self->{xml_lang};
  $self->{xml_lang}=$lang if $lang;
  $old;
}


sub base ($;$) {
  my($self,$base)=@_;
  my $old=$self->{xml_base};
  $self->{xml_base}=$base if $base;
  $old;
}


sub parent ($;$) {
  my($self,$parent)=@_;
  my $old=$self->{parent};
  $self->{parent}=$parent if $parent;
  $old;
}


sub add_namespace ($$$) {
  my($self,$uri,$prefix)=@_;
  push(@{$self->xml_namespaces}, [$uri,$prefix]);

  if($self->{namespace_URI} eq $uri) {
    $self->{prefix}=$prefix;
  }

  # Set all attributes with that ns uri to the prefix
  if(my $attrs=$self->{attributes}) {
    for my $attr (@$attrs) {
      if($attr->uri eq $uri) {
        $attr->prefix($prefix);
      }
    }
  }
}


sub add_attribute ($$) {
  my($self,$attr)=@_;

  # Set namespace of attribute if possible
  for my $ns (@{$self->{namespaces}}) {
    my($uri,$prefix)=@$ns;
    if($attr->uri eq $uri) {
      $attr->prefix($prefix);
      last;
    }
  }
  push(@{$self->{attributes}}, $attr);
}


sub add_child ($$) {
  my($self,$child)=@_;

  # Set namespace of child if possible
  for my $ns (@{$self->{namespaces}}) {
    my($uri,$prefix)=@$ns;
    if($child->uri && 
       ($child->uri eq $uri)) {
      $child->prefix($prefix);
      last;
    }
  }
  $child->parent($self);
  push(@{$self->{children}}, $child);
}


sub emit ($) {
  my $self=shift;

  my $str='';
  $str.= "<".$self->{prefix}.":".$self->{name};
  if(my $namespaces=$self->{inscope_namespaces}) {
    for my $ns (@$namespaces) {
      my($prefix, $uri, $depth)=@$ns;
      $str.=qq{ xmlns:$prefix="$uri"}
        if $depth == $self->{depth};
    }
  }

  if(my $lang=$self->{xml_lang}) {
    $str.=qq{ xml:lang="$lang"};
  }

  if(my $base=$self->{xml_base}) {
    $str.=qq{ xml:base="$base"};
  }

  if(my $attrs=$self->{attributes}) {
    for my $attr (@$attrs) {
      $str.=" ".$attr->emit;
    }
  }

  if(my $children=$self->{children}) {
    $str.= ">";
    for my $child (@$children) {
      $str.= $child->emit;
    }
    $str.= "</".$self->{prefix}.":".$self->{name}.">";
  } else {
    $str .= "/>";
  }
  $str;
}


package Attribute;

sub new ($$$$) {
  my($proto,$ns_uri,$local_name,$value)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {
    namespace_URI => $ns_uri,
    name => $local_name,
    value => $value,
  };

  bless ($self, $class);
  return $self;
}


sub emit ($) {
  my($self)=shift;
  my $attr=$self->{prefix}.":".$self->{name};
  my $value=$self->{value};
  return qq{$attr="$value"};
}


sub uri ($;$) {
  my($self,$uri)=@_;
  my $old=$self->{uri};
  $self->{uri}=$uri if $uri;
  $old;
}

sub prefix ($;$) {
  my($self,$prefix)=@_;
  my $old=$self->{prefix};
  $self->{prefix}=$prefix if $prefix;
  $old;
}



package Text;

sub new ($$) {
  my($proto,$text)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {
    text => $text,
    parent => undef,
  };

  bless ($self, $class);
  return $self;
}


sub emit ($) {
  my($self)=shift;
  return $self->{text};
}


sub parent ($;$) {
  my($self,$parent)=@_;
  my $old=$self->{parent};
  $self->{parent}=$parent if $parent;
  $old;
}


sub uri ($) {
  return undef;
}

