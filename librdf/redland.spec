# -*- RPM-SPEC -*-
# Note that this is NOT a relocatable package
%define name    redland
%define version 0.9.14
%define release SNAP
%define prefix  /usr

%define perlinstallsitearch /usr/lib/perl5/site_perl/5.8.0/i386-linux-thread-multi
%define perlman3dir /usr/share/man/man3

Summary:   Redland RDF Application Framework
Name:      %{name}
Version:   %{version}
Release:   %{release}
Prefix:    %{_prefix}
Copyright: LGPL/MPL
Group:     Development/Libraries
Source:    http://www.redland.opensource.ac.uk/dist/source/%{name}-%{version}.tar.gz
URL:       http://www.redland.opensource.ac.uk/
BuildRoot: /tmp/%{name}-%{version}
BuildRequires: libxml2-devel >= 2.5.0
BuildRequires: curl-devel
BuildRequires: raptor-devel >= 1.0.0
Packager:  Dave Beckett <Dave.Beckett@bristol.ac.uk>
Docdir: %{prefix}/doc
Requires:  libxml2
Requires:  curl
Requires:  raptor

%description

Redland is a library that provides a high-level interface for RDF
(Resource Description Framework) implemented in an object-based API.
It is modular and supports different RDF/XML parsers, storage
mechanisms and other elements. Redland is designed for applications
developers to provide RDF support in their applications as well as
for RDF developers to experiment with the technology.

%package devel
Summary: Libraries and header files for programs that use Redland.
Group: Development/Libraries
Requires: redland
Requires: raptor >= 1.0.0

%description devel
Header files for development with Redland

%package perl
Summary: Perl modules for the Redland RDF system
Group: Development/Libraries
Requires: redland

%description perl
The redland-perl package contains the parts of Redland that provide
an interface to Perl.

%prep
%setup

%build
# Force use of default python even if 2.0 is present
PYTHON=python
export PYTHON

if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%{prefix} --enable-release --sysconfdir=/etc --mandir=%{_mandir} --libdir=%{_libdir} --with-raptor=system
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix} --enable-release --sysconfdir=/etc --mandir=%{_mandir} --libdir=%{_libdir} --with-raptor=system
fi

%{__make} OPTIMIZE="$RPM_OPT_FLAGS"

cd perl

%{__perl} Makefile.PL PREFIX=$RPM_BUILD_ROOT/%{_prefix} SYSCONFDIR=$RPM_BUILD_ROOT/%{_sysconfdir} INST_PREFIX=%{_prefix} INST_SYSCONFDIR=%{_sysconfdir}
make

cd ..


%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%makeinstall prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT/etc libdir=$RPM_BUILD_ROOT%{_libdir} mandir=$RPM_BUILD_ROOT%{_mandir}

find $RPM_BUILD_ROOT -print | xargs chmod u+w


# FIXME
# Fix it so -L.. -llibrdf works from perl, python, ... subdirs
# Must find a way to do this better
# ln -s .libs/librdf.a librdf.a

cd perl

make prefix=$RPM_BUILD_ROOT%{prefix} exec_prefix=$RPM_BUILD_ROOT%{prefix} bindir=$RPM_BUILD_ROOT%{prefix}/bin sbindir=$RPM_BUILD_ROOT%{prefix}/sbin sysconfdir=$RPM_BUILD_ROOT/etc datadir=$RPM_BUILD_ROOT%{prefix}/share includedir=$RPM_BUILD_ROOT%{prefix}/include libdir=$RPM_BUILD_ROOT%{prefix}/lib libexecdir=$RPM_BUILD_ROOT%{prefix}/libexec localstatedir=$RPM_BUILD_ROOT/var sharedstatedir=$RPM_BUILD_ROOT%{prefix}/com mandir=$RPM_BUILD_ROOT%{prefix}/share/man infodir=$RPM_BUILD_ROOT%{prefix}/share/info install PREFIX=$RPM_BUILD_ROOT%{prefix} INSTALLMAN1DIR=$RPM_BUILD_ROOT%{prefix}/share/man/man1 INSTALLMAN3DIR=$RPM_BUILD_ROOT%{prefix}/share/man/man3 install

find $RPM_BUILD_ROOT -name perllocal.pod -exec rm -v {}  \;

cd ..

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(0555, bin, bin)
%{prefix}/bin/redland-config
%{prefix}/bin/redland-db-upgrade
%{prefix}/bin/rdfproc
%{prefix}/lib/librdf.a
%{prefix}/lib/librdf.so.*

%defattr (0444, bin, bin, 0555)
%{prefix}/lib/librdf.la
%{prefix}/lib/librdf.so

%doc AUTHORS COPYING COPYING.LIB ChangeLog
%doc README NEWS LICENSE.txt
%doc *.html

%doc %{_mandir}/man1/redland-config.1*
%doc %{_mandir}/man1/redland-db-upgrade.1*
%doc %{_mandir}/man1/rdfproc.1*
%doc %{_mandir}/man3/redland.3*

%files devel
%defattr(-, root, root)

%{prefix}/include/redland.h
%{prefix}/include/librdf.h 
%{prefix}/include/rdf_*.h
%doc docs/README.html
%doc docs/java.html docs/perl.html docs/php.html
%doc docs/python.html docs/ruby.html docs/tcl.html
%doc docs/api

%files perl
%defattr(-, bin, bin)

%{perlinstallsitearch}/auto
%{perlinstallsitearch}/RDF/Redland
%{perlinstallsitearch}/RDF/Redland.pm

%doc %{perlman3dir}/RDF::Redland*.3pm*

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%changelog
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
