#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010, 2011 Igalia S.L. All rights reserved.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src libs bindings po tools examples

if ENABLE_TESTS
SUBDIRS += tests
endif

if ENABLE_GTK_DOC
SUBDIRS += doc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = grilo-0.3.pc

if BUILD_GRILO_NET
pkgconfig_DATA += grilo-net-0.2.pc
endif

if BUILD_GRILO_PLS
pkgconfig_DATA += grilo-pls-0.2.pc
endif

dist_man1_MANS = grl-inspect-0.2.1 grl-launch-0.2.1

MAINTAINERCLEANFILES = \
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(srcdir)/gtk-doc.make \
	$(srcdir)/m4/gtk-doc.m4 \
	$(srcdir)/m4/intltool.m4 \
	$(srcdir)/omf.make \
	$(srcdir)/xmldocs.make

DISTCLEANFILES = $(MAINTAINERCLEANFILES)

EXTRA_DIST = ./m4/introspection.m4

DISTCHECK_CONFIGURE_FLAGS= \
   --enable-grl-net        \
   --enable-grl-pls        \
   --enable-gtk-doc        \
   --enable-introspection  \
   --enable-test-ui        \
   --enable-vala

distclean-local:
	-rm -rf autom4te.cache

dist-hook:

-include $(top_srcdir)/git.mk
-include $(top_srcdir)/release.mk
