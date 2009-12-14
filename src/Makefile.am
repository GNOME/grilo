#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

lib_LTLIBRARIES		 = libmediastore.la

libmediastore_la_CFLAGS	 = $(DEPS_CFLAGS)
libmediastore_la_LIBADD	 = $(DEPS_LIBS)

libmediastore_la_SOURCES = \
	media-plugin.c media-plugin.h media-plugin-priv.h \
	plugin-registry.c plugin-registry.h \
	metadata-source.c metadata-source.h \
	media-source.c media-source.h

noinst_HEADERS = \
	media-plugin-priv.h

MAINTAINERCLEANFILES	 = \
	*.in \
	*~
