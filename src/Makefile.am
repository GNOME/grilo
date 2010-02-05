#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = content

lib_LTLIBRARIES = lib@GRL_NAME@.la

lib@GRL_NAME@_la_CFLAGS =	\
	$(DEPS_CFLAGS)		\
	-I$(srcdir)		\
	-I$(srcdir)/content

lib@GRL_NAME@_la_LIBADD =	\
	$(DEPS_LIBS)		\
	$(builddir)/content/libcontent.la

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
