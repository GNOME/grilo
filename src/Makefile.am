#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = . tests

lib_LTLIBRARIES = lib@GRL_NAME@.la

lib@GRL_NAME@_la_CFLAGS =	\
	$(DEPS_CFLAGS)		\
	-I$(srcdir)		\
	-I$(srcdir)/data	\
	-DGRILO_COMPILATION

lib@GRL_NAME@_la_LIBADD =	\
	$(DEPS_LIBS)

lib@GRL_NAME@_la_SOURCES =					\
	grl-media-plugin.c grl-media-plugin-priv.h		\
	grl-plugin-registry.c					\
	grl-metadata-key.c grl-metadata-key-priv.h		\
	grl-metadata-source.c grl-metadata-source-priv.h	\
	grl-media-source.c					\
	grl-log.c

data_c_sources =		\
	data/grl-data.c		\
	data/grl-media.c	\
	data/grl-media-audio.c	\
	data/grl-media-video.c	\
        data/grl-media-image.c	\
	data/grl-media-box.c	\
	data/grl-config.c

lib@GRL_NAME@_la_SOURCES += $(data_c_sources)

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

data_h_headers =		\
	data/grl-data.h		\
	data/grl-media.h	\
	data/grl-media-box.h	\
	data/grl-media-audio.h	\
	data/grl-media-video.h	\
	data/grl-media-image.h	\
	data/grl-config.h

lib@GRL_NAME@inc_HEADERS += $(data_h_headers)

noinst_HEADERS =			\
	grl-media-plugin-priv.h		\
	grl-metadata-source-priv.h	\
	grl-metadata-key-priv.h

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(srcdir)/data

# introspection support
if HAVE_INTROSPECTION
introspection_sources = \
	$(lib@GRL_NAME@inc_HEADERS)	\
	$(lib@GRL_NAME@_la_SOURCES)

Grl-@GRL_MAJORMINOR@.gir: lib@GRL_NAME@.la
Grl_0_1_gir_INCLUDES = GObject-2.0
Grl_0_1_gir_CFLAGS = $(lib@GRL_NAME@_la_CFLAGS)
Grl_0_1_gir_LIBS = lib@GRL_NAME@.la
Grl_0_1_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
INTROSPECTION_GIRS += Grl-@GRL_MAJORMINOR@.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif
