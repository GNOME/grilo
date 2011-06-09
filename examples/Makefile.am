AM_CFLAGS = $(DEPS_CFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/src/data
LDADD = $(DEPS_LIBS) $(top_builddir)/src/lib@GRL_NAME@.la

noinst_PROGRAMS = browsing configuring-plugins efficient-metadata-resolution	\
	loading-plugins multivalues searching

browsing_SOURCES = browsing.c

configuring_plugins_SOURCES = configuring-plugins.c

efficient_metadata_resolution_SOURCES = efficient-metadata-resolution.c

loading_plugins_SOURCES = loading-plugins.c

multivalues_SOURCES = multivalues.c

searching_SOURCES = searching.c

MAINTAINERCLEANFILES = Makefile.in *~
DISTCLEANFILES = $(MAINTAINERCLEANFILES)
