#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = content

lib_LTLIBRARIES		 = libmedia-store.la

libmedia_store_la_CFLAGS = $(DEPS_CFLAGS)

libmedia_store_la_LIBADD = $(DEPS_LIBS) $(top_builddir)/src/content/libcontent.la

libmedia_store_la_SOURCES =				\
	ms-media-plugin.c ms-media-plugin-priv.h	\
	ms-plugin-registry.c				\
	ms-metadata-key.c				\
	ms-metadata-source.c ms-metadata-source-priv.h	\
	ms-media-source.c				\
	ms-log.c

libmedia_storeincdir	= $(includedir)/$(PACKAGE_NAME)

libmedia_storeinc_HEADERS =	\
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
