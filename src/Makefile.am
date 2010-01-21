#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = content

lib_LTLIBRARIES = lib@PACKAGE_NAME@-@MS_MAJORMINOR@.la

lib@PACKAGE_NAME@_@MS_MAJORMINOR@_la_CFLAGS =	\
	$(DEPS_CFLAGS)				\
	-I.					\
	-I./content

lib@PACKAGE_NAME@_@MS_MAJORMINOR@_la_LIBADD =	\
	$(DEPS_LIBS)				\
	content/libcontent.la

lib@PACKAGE_NAME@_@MS_MAJORMINOR@_la_SOURCES =		\
	ms-media-plugin.c ms-media-plugin-priv.h	\
	ms-plugin-registry.c				\
	ms-metadata-key.c				\
	ms-metadata-source.c ms-metadata-source-priv.h	\
	ms-media-source.c				\
	ms-log.c

lib@PACKAGE_NAME@_@MS_MAJORMINOR@incdir =	\
	$(includedir)/@PACKAGE_NAME@-@MS_MAJORMINOR@

lib@PACKAGE_NAME@_@MS_MAJORMINOR@inc_HEADERS =	\
	media-store.h				\
	ms-error.h				\
	ms-media-plugin.h			\
	ms-plugin-registry.h			\
	ms-metadata-key.h			\
	ms-metadata-source.h			\
	ms-media-source.h			\
	ms-log.h

noinst_HEADERS =			\
	ms-media-plugin-priv.h		\
	ms-metadata-source-priv.h

MAINTAINERCLEANFILES = \
	*.in \
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
