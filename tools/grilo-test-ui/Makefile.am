#
# Makefile.am
#
# Author: Iago Toral <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS = grilo-test-ui-@GRL_MAJORMINOR@

grilo_test_ui_@GRL_MAJORMINOR@_SOURCES =	\
	main.c											\
	flickr-auth.h									\
	flickr-auth.c

grilo_test_ui_@GRL_MAJORMINOR@_CFLAGS =	\
	-DPREFIX=$(prefix)							\
	$(GTU_CFLAGS)									\
	$(GRL_FLICKR_CFLAGS)							\
	-I$(top_srcdir)/src							\
	-I$(top_srcdir)/src/data

grilo_test_ui_@GRL_MAJORMINOR@_LDADD =		\
	$(DEPS_LIBS)									\
	$(GTU_LIBS)										\
	$(GRL_FLICKR_LIBS)							\
	$(top_builddir)/src/lib@GRL_NAME@.la

MAINTAINERCLEANFILES =	\
	*.in						\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
