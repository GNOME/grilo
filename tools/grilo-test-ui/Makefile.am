#
# Makefile.am
#
# Author: Iago Toral <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

bin_PROGRAMS = grilo-test-ui-@GRL_MAJORMINOR@

grilo_test_ui_@GRL_MAJORMINOR@_SOURCES =	\
	main.c

grilo_test_ui_@GRL_MAJORMINOR@_CFLAGS =	\
	$(DEPS_CFLAGS)									\
	$(GTU_CFLAGS)									\
	$(GRL_FLICKR_CFLAGS)							\
	-DPREFIX=$(prefix)							\
	-I$(top_srcdir)/src							\
	-I$(top_srcdir)/src/data

grilo_test_ui_@GRL_MAJORMINOR@_LDADD =		\
	$(DEPS_LIBS)									\
	$(GTU_LIBS)										\
	$(GRL_FLICKR_LIBS)							\
	$(top_builddir)/src/lib@GRL_NAME@.la

if HAVE_OAUTH

grilo_test_ui_@GRL_MAJORMINOR@_SOURCES +=	\
	flickr-oauth.h									\
	flickr-oauth.c

grilo_test_ui_@GRL_MAJORMINOR@_CFLAGS +=	\
	$(OAUTH_CFLAGS)

grilo_test_ui_@GRL_MAJORMINOR@_LDADD +=	\
	$(OAUTH_LIBS)

endif

-include $(top_srcdir)/git.mk
