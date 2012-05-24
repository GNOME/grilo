#
# Makefile.am
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS = grl-inspect-@GRL_MAJORMINOR@

grl_inspect_@GRL_MAJORMINOR@_SOURCES =	\
	grl-inspect.c

grl_inspect_@GRL_MAJORMINOR@_CFLAGS =	\
	-DPREFIX=$(prefix)						\
	-I$(top_srcdir)/src						\
	-I$(top_srcdir)/src/data

grl_inspect_@GRL_MAJORMINOR@_LDADD =	\
	$(DEPS_LIBS)								\
	$(GTK_LIBS)									\
	$(top_builddir)/src/lib@GRL_NAME@.la

MAINTAINERCLEANFILES =	\
	*.in						\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
