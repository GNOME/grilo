#
# Makefile.am
#
# Author: Juan A. Suárez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

noinst_LTLIBRARIES		 = libdata.la

libdata_la_CFLAGS =	\
	$(DEPS_CFLAGS)	\
	-I$(srcdir)	\
	-I$(srcdir)/..

libdata_la_LIBADD	 = $(DEPS_LIBS)

libdata_la_SOURCES =		\
	grl-data.c		\
	grl-data-media.c	\
	grl-data-audio.c	\
	grl-data-video.c	\
        grl-data-image.c	\
	grl-data-box.c	\
	grl-data-config.c

libdataincdir = $(includedir)/@GRL_NAME@

libdatainc_HEADERS =		\
	grl-data.h		\
	grl-data-media.h	\
	grl-data-box.h	\
	grl-data-audio.h	\
	grl-data-video.h	\
	grl-data-image.h	\
	grl-data-config.h

MAINTAINERCLEANFILES	 = \
	*.in \
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
