#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = content

lib_LTLIBRARIES		 = libmediastore.la

libmediastore_la_CFLAGS	 = $(DEPS_CFLAGS)

libmediastore_la_LIBADD	 = $(DEPS_LIBS) $(top_builddir)/src/content/libcontent.la

libmediastore_la_SOURCES =				\
	ms-media-plugin.c ms-media-plugin-priv.h	\
	ms-plugin-registry.c				\
	ms-metadata-key.c				\
	ms-metadata-source.c ms-metadata-source-priv.h	\
	ms-media-source.c				\
	ms-log.c

libmediastoreincdir	= $(includedir)/$(PACKAGE_NAME)

libmediastoreinc_HEADERS =	\
	media-store.h		\
	ms-error.h		\
	ms-media-plugin.h	\
	ms-plugin-registry.h	\
	ms-metadata-key.h	\
	ms-metadata-source.h	\
	ms-media-source.h	\
	ms-log.h

noinst_HEADERS =			\
	ms-media-plugin-priv.h		\
	ms-metadata-source-priv.h

MAINTAINERCLEANFILES = \
	*.in \
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
