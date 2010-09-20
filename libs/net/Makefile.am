#
# Makefile.am
#
# Author: Víctor M. Jáquez L. <vjaquez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

if BUILD_GRILO_NET
lib_LTLIBRARIES = libgrlnet-@GRL_MAJORMINOR@.la
noinst_PROGRAMS = wc-test
endif

libgrlnet_@GRL_MAJORMINOR@_la_DEPENDENCIES =	\
	$(top_builddir)/src/lib@GRL_NAME@.la

libgrlnet_@GRL_MAJORMINOR@_la_SOURCES = \
	grl-net-wc.c

libgrlnet_@GRL_MAJORMINOR@_la_CFLAGS =	\
	-I $(top_srcdir)/src		\
	-I $(top_srcdir)/src/data	\
	$(DEPS_CFLAGS)			\
	$(NET_CFLAGS)

libgrlnet_@GRL_MAJORMINOR@_la_LIBADD =	\
	$(top_builddir)/src/lib@GRL_NAME@.la	\
	$(DEPS_LIBS)				\
	$(NET_LIBS)

libgrlnet_@GRL_MAJORMINOR@includedir =	\
	$(includedir)/@GRL_NAME@/net

libgrlnet_@GRL_MAJORMINOR@include_HEADERS =	\
	grl-net-wc.h

wc_test_SOURCES = 	\
	wc-test.c

wc_test_CFLAGS = 	\
	$(DEPS_CFLAGS)	\
	-D_GNU_SOURCE

wc_test_LDADD =						\
	$(top_builddir)/libs/net/libgrlnet-@GRL_MAJORMINOR@.la	\
	$(DEPS_LIBS)

MAINTAINERCLEANFILES =	\
	*.in		\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
