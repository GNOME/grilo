#
# Makefile.am
#
# Author: Juan A. Suárez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

lib_LTLIBRARIES		 = libcontent.la

libcontent_la_CFLAGS	 = $(DEPS_CFLAGS)
libcontent_la_LIBADD	 = $(DEPS_LIBS)

libcontent_la_SOURCES = \
	ms-content.c ms-content.h \
	ms-content-media.c ms-content-media.h \
	ms-content-audio.c ms-content-audio.h \
	ms-content-video.c ms-content-video.h

MAINTAINERCLEANFILES	 = \
	*.in \
	*~
