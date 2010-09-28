#
# Makefile.am
#
# Author: Víctor M. Jáquez L. <vjaquez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

if BUILD_GRILO_NET
lib_LTLIBRARIES = libgrlnet-@GRL_MAJORMINOR@.la
noinst_PROGRAMS = wc-test
endif

libgrlnet_@GRL_MAJORMINOR@_la_DEPENDENCIES =	\
	$(top_builddir)/src/lib@GRL_NAME@.la

libgrlnet_@GRL_MAJORMINOR@_la_SOURCES = \
	grl-net-wc.c

libgrlnet_@GRL_MAJORMINOR@_la_CFLAGS =	\
	-I $(top_srcdir)/src		\
	-I $(top_srcdir)/src/data	\
	$(DEPS_CFLAGS)			\
	$(NET_CFLAGS)

libgrlnet_@GRL_MAJORMINOR@_la_LIBADD =	\
	$(top_builddir)/src/lib@GRL_NAME@.la	\
	$(DEPS_LIBS)				\
	$(NET_LIBS)

libgrlnet_@GRL_MAJORMINOR@includedir =	\
	$(includedir)/@GRL_NAME@/net

libgrlnet_@GRL_MAJORMINOR@include_HEADERS =	\
	grl-net-wc.h				\
	grl-net.h

wc_test_SOURCES = 	\
	wc-test.c

wc_test_CFLAGS = 	\
	$(DEPS_CFLAGS)	\
	-D_GNU_SOURCE

wc_test_LDADD =						\
	$(top_builddir)/libs/net/libgrlnet-@GRL_MAJORMINOR@.la	\
	$(DEPS_LIBS)

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
CLEANFILES =

# introspection support
if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
gir_headers = $(patsubst %,$(srcdir)/%, $(libgrlnet_@GRL_MAJORMINOR@include_HEADERS))
gir_sources = $(patsubst %,$(srcdir)/%, $(libgrlnet_@GRL_MAJORMINOR@_la_SOURCES))

INTROSPECTION_GIRS =

introspection_sources =	\
	$(gir_headers)	\
	$(gir_sources)

GrlNet-@GRL_MAJORMINOR@.gir: libgrlnet-@GRL_MAJORMINOR@.la
GrlNet_@GRL_MAJORMINOR_NORM@_gir_INCLUDES = GObject-2.0 Gio-2.0
GrlNet_@GRL_MAJORMINOR_NORM@_gir_CFLAGS = -I$(top_srcdir)/libs $(libgrlnet_@GRL_MAJORMINOR@_la_CFLAGS)
GrlNet_@GRL_MAJORMINOR_NORM@_gir_LIBS = libgrlnet-@GRL_MAJORMINOR@.la
GrlNet_@GRL_MAJORMINOR_NORM@_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += GrlNet-@GRL_MAJORMINOR@.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif
