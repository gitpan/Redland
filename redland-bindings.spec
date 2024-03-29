# -*- RPM-SPEC -*-
# Note that this is NOT a relocatable package
%define name    redland-bindings
%define version 1.0.13.1
%define release SNAP
%define prefix  /usr

%define redland_version 1.0.13

# Installation directories for perl, python and ruby are host-specific
%define perl_sitearch	%(eval "`perl -V:installsitearch`"; echo $installsitearch)
# Not %{_libdir} as on FC4 x86_64 it is the wrong place
%define python_libdir   %{_prefix}/lib
%define ruby_dir	%(ruby -rrbconfig -e "puts Config::CONFIG['archdir']")
%define ruby_libdir	%(ruby -rrbconfig -e "puts Config::CONFIG['rubylibdir']")

Summary:   Redland librdf Language Bindings
Name:      %{name}
Version:   %{version}
Release:   %{release}
Prefix:    %{_prefix}
License:   LGPL/Apache 2
Group:     Development/Libraries
Source:    http://download.librdf.org/source/%{name}-%{version}.tar.gz
URL:       http://librdf.org/
BuildRoot: /tmp/%{name}-%{version}
BuildRequires: redland-devel >= %{redland_version}
BuildRequires: perl >= 5.8.0
BuildRequires: python-devel >= 2.3.0
BuildRequires: ruby >= 1.8.0
BuildRequires: ruby-devel >= 1.8.0
Packager:  Dave Beckett <dave@dajobe.org>
Docdir: %{prefix}/doc
Requires:  redland >= %{redland_version}
Requires:  ruby >= 1.8.0

%description

Redland librdf Language bindings provide high-level interfaces for
RDF (Resource Description Framework) implemented on top of the
Redland librdf library which provides an object-based RDF API,
supports for syntaxes and triple stores.

%package -n redland-perl
Summary: Perl modules for the Redland librdf RDF library
Group: Development/Libraries
Requires: redland = %{redland_version}

%description -n redland-perl
The redland-perl package contains the parts of Redland librdf that provide
an interface to Perl.

%package -n redland-python
Summary: Python modules for the Redland librdf RDF library
Group: Development/Libraries
Requires: redland = %{redland_version}

%description -n redland-python
The redland-python package contains the parts of Redland librdf that provide
an interface to Python.

%package -n redland-ruby
Summary: Ruby modules for the Redland librdf RDF library
Group: Development/Libraries
Requires: redland = %{redland_version}

%description -n redland-ruby
The redland-ruby package contains the parts of Redland librdf that provide
an interface to Ruby.

%prep
%setup

%build
# Force use of default python
PYTHON=python
export PYTHON

%configure --enable-release

cd perl

%{__make} Makefile.perl PERL_MAKEMAKER_ARGS="PREFIX=$RPM_BUILD_ROOT/%{_prefix} SYSCONFDIR=$RPM_BUILD_ROOT/%{_sysconfdir} INST_PREFIX=%{_prefix} INST_SYSCONFDIR=%{_sysconfdir}"
%{__make}

cd ..


cd python

%{__make}

cd ..


cd ruby

%{__make}

cd ..


%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT


cd perl

%{__make} prefix=$RPM_BUILD_ROOT%{prefix} exec_prefix=$RPM_BUILD_ROOT%{prefix} bindir=$RPM_BUILD_ROOT%{prefix}/bin sbindir=$RPM_BUILD_ROOT%{prefix}/sbin sysconfdir=$RPM_BUILD_ROOT/etc datadir=$RPM_BUILD_ROOT%{prefix}/share includedir=$RPM_BUILD_ROOT%{prefix}/include libdir=$RPM_BUILD_ROOT%{_libdir} libexecdir=$RPM_BUILD_ROOT%{prefix}/libexec localstatedir=$RPM_BUILD_ROOT/var sharedstatedir=$RPM_BUILD_ROOT%{prefix}/com mandir=$RPM_BUILD_ROOT%{_mandir} infodir=$RPM_BUILD_ROOT%{prefix}/share/info install

find $RPM_BUILD_ROOT -name perllocal.pod -exec rm -v {}  \;

find $RPM_BUILD_ROOT%{perl_sitearch} -name \*.so -exec chmod 755 {} \;

cd ..

cd python

%{__make} install DESTDIR=$RPM_BUILD_ROOT
find $RPM_BUILD_ROOT%{prefix} -name "*.py[co]" -exec rm -f {} \;

cd ..

cd ruby

mkdir -p $RPM_BUILD_ROOT%{ruby_dir}
mkdir -p $RPM_BUILD_ROOT%{ruby_libdir}
%{__make} install DESTDIR=$RPM_BUILD_ROOT

cd ..

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files -n redland-perl
%defattr(-, bin, bin)

%{perl_sitearch}/auto
%{perl_sitearch}/RDF/Redland
%{perl_sitearch}/RDF/Redland.pm

%doc AUTHORS COPYING COPYING.LIB ChangeLog LICENSE.txt NEWS README
%doc LICENSE-2.0.txt NOTICE

%doc %{_mandir}/man3/RDF::Redland*.3pm*
%doc perl/example.pl

%files -n redland-python
%defattr(-, bin, bin)

%{python_libdir}/python*/site-packages/RDF.py
%{python_libdir}/python*/site-packages/Redland.so*

%doc AUTHORS COPYING COPYING.LIB LICENSE-2.0.txt
%doc README NEWS LICENSE.txt NOTICE

%doc docs/pydoc/RDF.html python/example.py

%files -n redland-ruby
%defattr(-, bin, bin)

%{ruby_dir}/redland.so
%{ruby_libdir}/rdf/*.rb
%{ruby_libdir}/rdf/redland/*.rb
%{ruby_libdir}/rdf/redland/schemas/*.rb

%doc AUTHORS COPYING COPYING.LIB LICENSE-2.0.txt
%doc README NEWS LICENSE.txt NOTICE

%doc ruby/example.rb

%post

%postun

%changelog
* Sat Apr 29 2006  Dave Beckett <dave@dajobe.org>
- Require ruby since the rpm file needs it

* Thu Aug 11 2005  Dave Beckett <dave.beckett@bristol.ac.uk>
- Define perl_sitearch, python_libdir, ruby_dir and ruby_libdir at
  rpm build time.
- Split build requirements out and declare minimum versions
- Use %configure and %{__make}

* Wed Nov 3 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- Added redland-ruby include wrapper classes
- BuildRequires: perl, python-devel, ruby-devel

* Mon Nov 2 2004   Dave Beckett <dave.beckett@bristol.ac.uk>
- License now LGPL/Apache 2
- Added LICENSE-2.0.txt and NOTICE

* Mon Jul 19 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- split redland to give redland-bindings
- requires redland 0.9.17

* Mon Jul 12 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- put /usr/share/redland/Redland.i in redland-devel

* Wed May  5 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- require raptor 1.3.0
- require rasqal 0.2.0

* Fri Jan 30 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- require raptor 1.2.0
- update for removal of python distutils
- require python 2.2.0+
- require perl 5.8.0+
- build and require mysql
- do not build and require threestore

* Sun Jan 4 2004  Dave Beckett <dave.beckett@bristol.ac.uk>
- added redland-python package
- export some more docs

* Mon Dec 15 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- require raptor 1.1.0
- require libxml 2.4.0 or newer
- added pkgconfig redland.pc
- split redland/devel package shared libs correctly

* Mon Sep 8 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- require raptor 1.0.0
 
* Thu Sep 4 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- added rdfproc
 
* Thu Aug 28 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- patches added post 0.9.13 to fix broken perl UNIVERSAL::isa
 
* Thu Aug 21 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- Add redland-db-upgrade.1
- Removed duplicate perl CORE shared objects

* Sun Aug 17 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- Updates for new perl module names.

* Tue Apr 22 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- Updated for Redhat 9, RPM 4

* Fri Feb 12 2003 Dave Beckett <dave.beckett@bristol.ac.uk>
- Updated for redland 0.9.12

* Fri Jan 4 2002 Dave Beckett <dave.beckett@bristol.ac.uk>
- Updated for new Perl module names

* Fri Sep 14 2001 Dave Beckett <dave.beckett@bristol.ac.uk>
- Added shared libraries
