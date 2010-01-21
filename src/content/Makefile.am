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
	ms-content.c		\
	ms-content-media.c	\
	ms-content-audio.c	\
	ms-content-video.c	\
        ms-content-image.c	\
	ms-content-box.c

libcontentincdir = $(includedir)/@PACKAGE_NAME@-@MS_MAJORMINOR@

libcontentinc_HEADERS =		\
	ms-content.h		\
	ms-content-media.h	\
	ms-content-box.h	\
	ms-content-audio.h	\
	ms-content-video.h	\
	ms-content-image.h

MAINTAINERCLEANFILES	 = \
	*.in \
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
