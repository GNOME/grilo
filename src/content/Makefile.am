#
# Makefile.am
#
# Author: Juan A. Suárez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

noinst_LTLIBRARIES		 = libcontent.la

libcontent_la_CFLAGS =	\
	$(DEPS_CFLAGS)	\
	-I$(srcdir)	\
	-I$(srcdir)/..

libcontent_la_LIBADD	 = $(DEPS_LIBS)

libcontent_la_SOURCES =		\
	grl-content.c		\
	grl-content-media.c	\
	grl-content-audio.c	\
	grl-content-video.c	\
        grl-content-image.c	\
	grl-content-box.c

libcontentincdir = $(includedir)/@GRL_NAME@

libcontentinc_HEADERS =		\
	grl-content.h		\
	grl-content-media.h	\
	grl-content-box.h	\
	grl-content-audio.h	\
	grl-content-video.h	\
	grl-content-image.h

MAINTAINERCLEANFILES	 = \
	*.in \
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
