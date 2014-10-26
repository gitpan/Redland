# -*- Mode: Perl -*-
#
# BlankNode.pm - Redland Perl RDF Blank Node module
#
# $Id: BlankNode.pm 10593 2006-03-05 08:30:38Z dajobe $
#
# Copyright (C) 2005 David Beckett - http://purl.org/net/dajobe/
# Copyright (C) 2005 University of Bristol - http://www.bristol.ac.uk/
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

package RDF::Redland::BlankNode;

use strict;

use vars qw(@ISA);

@ISA='RDF::Redland::Node';


=pod

=head1 NAME

RDF::Redland::BlankNode - Redland RDF Blank Node Class

=head1 SYNOPSIS

  use RDF::Redland;
  my $node1=new RDF::Redland::BlankNode("id");

=head1 DESCRIPTION

This class represents a blank nodes in the RDF graph.  See
L<RDF::Redland::Node> for the methods on this object.

=cut

######################################################################

=pod

=head1 CONSTRUCTOR

=over

=item new IDENTIFIER

Create a new blank node with identifier I<IDENTIFIER>.

=cut

# CONSTRUCTOR
sub new ($$) {
  my($proto,$arg)=@_;
  my $class = ref($proto) || $proto;
  my $self  = {};

  return RDF::Redland::Node->new_from_blank_identifier($arg);
}

=back

=head1 SEE ALSO

L<RDF::Redland::Node>

=head1 AUTHOR

Dave Beckett - http://purl.org/net/dajobe/

=cut

1;
