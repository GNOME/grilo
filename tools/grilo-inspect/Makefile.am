#
# Makefile.am
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.

grl-inspect.c: grl-core-keys.h

grl-core-keys.h: $(top_builddir)/src/grl-metadata-key.h
	$(AWK) 'BEGIN { printf "gchar *grl_core_keys[] = {\n" } { if ( $$2 ~ /^GRL_METADATA_KEY_/ && $$2 !~ /_GET_/ ) printf "\""$$2"\",\n" } END { printf "};\n" }' $< > $@

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS =			\
	grl-inspect-@GRL_MAJORMINOR@

grl_inspect_@GRL_MAJORMINOR@_SOURCES =	\
	grl-inspect.c								\
	grl-core-keys.h

grl_inspect_@GRL_MAJORMINOR@_CFLAGS =	\
	-DPREFIX=$(prefix)						\
	-I$(top_srcdir)/src						\
	-I$(top_srcdir)/src/data

grl_inspect_@GRL_MAJORMINOR@_LDADD =	\
	$(DEPS_LIBS)								\
	$(GTK_LIBS)									\
	$(top_builddir)/src/lib@GRL_NAME@.la

noinst_HEADERS = grl-core-keys.h

MAINTAINERCLEANFILES =	\
	grl-core-keys.h		\
	*.in						\
	*~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
