#!/usr/bin/perl -pi~
#
# Perl script to update the Redland Perl modules to 0.9.11 names
#
# $Id: update-perl-api.pl,v 1.2 2002/06/21 17:29:27 cmdjb Exp $
#
# USAGE: update-perl-api.pl <perl script names>
#
#

# Uses
s%use RDF;%use RDF::Redland;%;
s%use RDF::RSS;%use RDF::Redland::RSS;%;

# Classes

s%RDF::Node%RDF::Redland::Node%g;
s%RDF::Statement%RDF::Redland::Statement%g;
s%RDF::Model%RDF::Redland::Model%g;
s%RDF::Storage%RDF::Redland::Storage%g;
s%RDF::Stream%RDF::Redland::Stream%g;
s%RDF::Parser%RDF::Redland::Parser%g;
s%RDF::Iterator%RDF::Redland::Iterator%g;
s%RDF::URI%RDF::Redland::URI%g;
s%RDF::RSS%RDF::Redland::RSS%g;
