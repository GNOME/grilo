#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = content

lib_LTLIBRARIES = lib@MS_NAME@.la

lib@MS_NAME@_la_CFLAGS =	\
	$(DEPS_CFLAGS)		\
	-I$(srcdir)		\
	-I$(srcdir)/content

lib@MS_NAME@_la_LIBADD =	\
	$(DEPS_LIBS)		\
	$(builddir)/content/libcontent.la

lib@MS_NAME@_la_SOURCES =				\
	ms-media-plugin.c ms-media-plugin-priv.h	\
	ms-plugin-registry.c				\
	ms-metadata-key.c				\
	ms-metadata-source.c ms-metadata-source-priv.h	\
	ms-media-source.c				\
	ms-log.c

lib@MS_NAME@incdir =	\
	$(includedir)/@MS_NAME@

lib@MS_NAME@inc_HEADERS =	\
	media-store.h		\
	ms-error.h		\
	ms-media-plugin.h	\
	ms-plugin-registry.h	\
	ms-metadata-key.h	\
	ms-metadata-source.h	\
	ms-media-source.h	\
	ms-log.h

noinst_HEADERS =		\
	ms-media-plugin-priv.h	\
	ms-metadata-source-priv.h

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
