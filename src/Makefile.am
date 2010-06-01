#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

lib_LTLIBRARIES = lib@GRL_NAME@.la

lib@GRL_NAME@_la_CFLAGS =	\
	$(DEPS_CFLAGS)		\
	-I$(srcdir)		\
	-I$(srcdir)/data

lib@GRL_NAME@_la_LIBADD =	\
	$(DEPS_LIBS)

lib@GRL_NAME@_la_SOURCES =					\
	grl-media-plugin.c grl-media-plugin-priv.h		\
	grl-plugin-registry.c					\
	grl-metadata-key.c					\
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

noinst_HEADERS =		\
	grl-media-plugin-priv.h	\
	grl-metadata-source-priv.h

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
CLEANFILES =
