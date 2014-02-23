#
# Makefile.am
#
# Author:  Mateu Batle <mateu.batle@collabora.com>
#
# Copyright (C) 2013 Collabora Ltd. All rights reserved.


lib_LTLIBRARIES = libgrlpls-@GRL_MAJORMINOR@.la

libgrlpls_@GRL_MAJORMINOR@_la_DEPENDENCIES =	\
	$(top_builddir)/src/lib@GRL_NAME@.la

libgrlpls_@GRL_MAJORMINOR@_la_SOURCES = \
	grl-pls.c

libgrlpls_@GRL_MAJORMINOR@_la_CFLAGS =	\
	-I $(top_srcdir)/src		\
	-I $(top_srcdir)/src/data	\
	-DLOCALEDIR=\"$(localedir)\" \
	$(DEPS_CFLAGS)			\
	$(TOTEM_PL_PARSER_CFLAGS)

libgrlpls_@GRL_MAJORMINOR@_la_LIBADD =	\
	$(top_builddir)/src/lib@GRL_NAME@.la	\
	$(DEPS_LIBS)				\
	$(TOTEM_PL_PARSER_LIBS)

libgrlpls_@GRL_MAJORMINOR@_la_LDFLAGS =	\
	-version-info $(GRLPLS_LT_VERSION)		\
	-no-undefined

libgrlpls_@GRL_MAJORMINOR@includedir =	\
	$(includedir)/@GRL_NAME@/pls

libgrlpls_@GRL_MAJORMINOR@include_HEADERS =	\
	grl-pls.h

CLEANFILES = *.gir

# introspection support
if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
gir_headers = $(patsubst %,$(srcdir)/%, $(libgrlpls_@GRL_MAJORMINOR@include_HEADERS))
gir_sources = $(patsubst %,$(srcdir)/%, $(libgrlpls_@GRL_MAJORMINOR@_la_SOURCES))

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all --add-include-path=$(top_srcdir)/src
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_srcdir)/src --includedir=$(top_builddir)/src

introspection_sources =	\
	$(gir_headers)	\
	$(gir_sources)

GrlPls-@GRL_MAJORMINOR@.gir: libgrlpls-@GRL_MAJORMINOR@.la
GrlPls_@GRL_MAJORMINOR_NORM@_gir_INCLUDES = GObject-2.0 Gio-2.0
GrlPls_@GRL_MAJORMINOR_NORM@_gir_SCANNERFLAGS = --include-uninstalled=$(top_builddir)/src/Grl-@GRL_MAJORMINOR@.gir
GrlPls_@GRL_MAJORMINOR_NORM@_gir_CFLAGS = -I $(top_srcdir)/src \
	-I $(top_srcdir)/src/data -I $(top_srcdir)/libs
GrlPls_@GRL_MAJORMINOR_NORM@_gir_LIBS = libgrlpls-@GRL_MAJORMINOR@.la \
	$(top_builddir)/src/lib@GRL_NAME@.la
GrlPls_@GRL_MAJORMINOR_NORM@_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += GrlPls-@GRL_MAJORMINOR@.gir

girdir = @INTROSPECTION_GIRDIR@
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = @INTROSPECTION_TYPELIBDIR@
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif
