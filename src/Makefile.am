#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = data

lib_LTLIBRARIES = lib@GRL_NAME@.la

lib@GRL_NAME@_la_CFLAGS =	\
	$(DEPS_CFLAGS)		\
	-I$(srcdir)		\
	-I$(srcdir)/data

lib@GRL_NAME@_la_LIBADD =	\
	$(DEPS_LIBS)		\
	$(builddir)/data/libdata.la

lib@GRL_NAME@_la_SOURCES =					\
	grl-media-plugin.c grl-media-plugin-priv.h		\
	grl-plugin-registry.c					\
	grl-metadata-key.c					\
	grl-metadata-source.c grl-metadata-source-priv.h	\
	grl-media-source.c					\
	grl-log.c

lib@GRL_NAME@incdir =	\
	$(includedir)/@GRL_NAME@

lib@GRL_NAME@inc_HEADERS =	\
	grilo.h			\
	grl-error.h		\
	grl-media-plugin.h	\
	grl-plugin-registry.h	\
	grl-metadata-key.h	\
	grl-metadata-source.h	\
	grl-media-source.h	\
	grl-log.h

noinst_HEADERS =		\
	grl-media-plugin-priv.h	\
	grl-metadata-source-priv.h

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
CLEANFILES =

# introspection support
if HAVE_INTROSPECTION
BUILT_GIRSOURCES =

INCLUDES =			\
	-I$(top_srcdir)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/data

@GRL_GIR_API_NAME@.gir: $(INTROSPECTION_SCANNER) lib@GRL_NAME@.la
	$(INTROSPECTION_SCANNER) -v			\
		--namespace @GRL_GIR_NAMESPACE@		\
		--nsversion=@GRL_MAJORMINOR@		\
		$(INCLUDES)				\
		--c-include="$(srcdir)/grilo.h"		\
		--include=GObject-2.0			\
		--library=lib@GRL_NAME@.la		\
		--libtool="$(top_builddir)/libtool"	\
	        --output $@				\
		$(lib@GRL_NAME@inc_HEADERS)		\
		$(lib@GRL_NAME@_la_SOURCES)		\
		data/*.h

BUILT_GIRSOURCES += @GRL_GIR_API_NAME@.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/

typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	LD_LIBRARY_PATH=.libs$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH} \
	$(INTROSPECTION_COMPILER) \
		--includedir=$(srcdir) \
		$(INTROSPECTION_COMPILER_OPTS) $< -o $(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif
