#
# Makefile.am
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS = grl-inspect

grl_inspect_SOURCES =	\
	grl-inspect.c

grl_inspect_CFLAGS =		\
	-DPREFIX=$(prefix)	\
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/data

grl_inspect_LDADD =	\
	-ldl -lpthread	\
	$(DEPS_LIBS)	\
	$(GTK_LIBS)	\
	$(top_builddir)/src/lib@GRL_NAME@.la

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
