#!/usr/bin/python
#
# setup.py - Setup script for the Python interface to Redland
#
# $Id: setup.py,v 1.16 2003/08/26 19:44:06 cmdjb Exp $
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

"""Setup script for the Python interface to Redland"""

import os, string, re
from distutils.core import setup, Extension


__revision__ = "$Id: setup.py,v 1.16 2003/08/26 19:44:06 cmdjb Exp $"


if os.environ.has_key('TOP_SRCDIR'):
  top_srcdir=os.environ['TOP_SRCDIR']
else:
  top_srcdir=os.pardir

if os.environ.has_key('PACKAGE'):
  package=os.environ['PACKAGE']
else:
  package="Redland"

if os.environ.has_key('VERSION'):
  version=os.environ['VERSION']
else:
  version='?.?'

print "Using source dir", top_srcdir, "to build package", package, version

# Next 13 lines were 2 in perl
config=os.path.join("..", "redland-src-config")
if os.access(config, os.R_OK):
  fh=os.popen(config + " --libs", 'r')
  redland_libs=""
  while 1:
    bit=fh.read()
    if not bit:
      break
    redland_libs=redland_libs + bit
  fh.close
  redland_libs=string.strip(redland_libs)
else:
  redland_libs=""

define_macros=[("HAVE_CONFIG", "1")]

def makelibname(name):
  return "lib"+name+".a"

# Convert -Ldir -lname into lists of dirs, names
# python will still probably link with the wrong versions since it
# is impossible to list full path names for libraries.
regex1=re.compile(r"-L(\S+) -l(\S+)\s*")
regex2=re.compile(r"-l(\S+)\s*")

library_dirs=[os.path.join(os.pardir, "librdf", ".libs"),
              os.path.join(os.pardir, "raptor", ".libs")]
include_dirs=[os.path.join(top_srcdir, "librdf")]
libraries=["rdf","raptor"]
t=redland_libs

while t:
  match=regex1.match(t)
  if match:
    dir=match.group(1)
    name=match.group(2)
    t=re.sub(regex1,"",t,1)
    if name != "rdf" and name != "raptor": # save all except rdf, raptor
      library_dirs=library_dirs + [dir]
      libraries=libraries + [name]
    continue

  match=regex2.match(t)
  if match:
    name=match.group(1)
    t=re.sub(regex2,"",t,1)
    if name != "rdf" and name != "raptor": # save all except librdf/raptor.a
      libraries=libraries + [name]
    continue

  if t:
    print "Ignoring extra stuff in redland-config output: "+t
  break



setup (name = package,
       version = version,
       description = "Redland RDF library Python interface",
       license = "LGPL / MPL",
       author = "Dave Beckett",
       author_email = "Dave.Beckett@bristol.ac.uk",
       url = "http://www.redland.opensource.ac.uk/",

       ext_modules = [Extension("Redland",
                                ["Redland_wrap.c"], 
                                include_dirs = include_dirs,
                                define_macros= define_macros,
                                library_dirs = library_dirs,
                                libraries    = libraries
                               )
                     ],
	py_modules = ["RDF"]
)
