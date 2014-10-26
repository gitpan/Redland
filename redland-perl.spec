# -*- RPM-SPEC -*-
# Note that this is NOT a relocatable package
%define name    redland-perl
%define version 1.0.5.3
%define release SNAP
%define prefix  /usr

%define redland_version 1.0.5

# Installation directories for perl, python and ruby are host-specific
%define perl_sitearch	%(eval "`perl -V:installsitearch`"; echo $installsitearch)

Summary:   Redland RDF Application Framework API Bindings
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
Packager:  Dave Beckett <dave@dajobe.org>
Docdir: %{prefix}/doc
Requires:  redland >= %{redland_version}
Requires:  ruby >= 1.8.0

%description

Redland is a library that provides a high-level interface for RDF
(Resource Description Framework) implemented in an object-based API.
It is modular and supports different RDF/XML parsers, storage
mechanisms and other elements. Redland is designed for applications
developers to provide RDF support in their applications as well as
for RDF developers to experiment with the technology.

%package -n redland-perl
Summary: Perl modules for the Redland RDF library
Group: Development/Libraries
Requires: redland = %{redland_version}

%description -n redland-perl
The redland-perl package contains the parts of Redland that provide
an interface to Perl.

%prep
%setup

%build

%configure --enable-release

cd perl

%{__make} Makefile.perl MAKE_PL_OPTS="PREFIX=$RPM_BUILD_ROOT/%{_prefix} SYSCONFDIR=$RPM_BUILD_ROOT/%{_sysconfdir} INST_PREFIX=%{_prefix} INST_SYSCONFDIR=%{_sysconfdir}"
%{__make}

cd ..

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT


cd perl

%{__make} prefix=$RPM_BUILD_ROOT%{prefix} exec_prefix=$RPM_BUILD_ROOT%{prefix} bindir=$RPM_BUILD_ROOT%{prefix}/bin sbindir=$RPM_BUILD_ROOT%{prefix}/sbin sysconfdir=$RPM_BUILD_ROOT/etc datadir=$RPM_BUILD_ROOT%{prefix}/share includedir=$RPM_BUILD_ROOT%{prefix}/include libdir=$RPM_BUILD_ROOT%{_libdir} libexecdir=$RPM_BUILD_ROOT%{prefix}/libexec localstatedir=$RPM_BUILD_ROOT/var sharedstatedir=$RPM_BUILD_ROOT%{prefix}/com mandir=$RPM_BUILD_ROOT%{_mandir} infodir=$RPM_BUILD_ROOT%{prefix}/share/info install

find $RPM_BUILD_ROOT -name perllocal.pod -exec rm -v {}  \;

find $RPM_BUILD_ROOT%{perl_sitearch} -name \*.so -exec chmod 755 {} \;

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

%post

%postun

%changelog
* Mon Nov 27 2006  Dave Beckett <dave@dajobe.org>
- redland-perl RPM

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
