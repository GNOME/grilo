#
# Makefile.am
#
# Author: Iago Toral <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS = test-ui

test_ui_SOURCES =	\
	main.c

test_ui_CFLAGS =		\
	-DPREFIX=$(prefix)	\
	$(GTK_CFLAGS)		\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/content

test_ui_LDADD =		\
	-ldl -lpthread	\
	$(DEPS_LIBS)	\
	$(GTK_LIBS)	\
	$(top_builddir)/src/lib@MS_NAME@.la

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
