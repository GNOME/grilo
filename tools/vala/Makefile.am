bin_PROGRAMS = grilo-simple-playlist

BUILT_SOURCES = vala.stamp

# We don't want -Werror by now
CFLAGS = -Wall -std=c99

grilo_simple_playlist_VALASOURCES = grilo-test.vala

grilo_simple_playlist_SOURCES = 			\
	vala.stamp 					\
	$(grilo_simple_playlist_VALASOURCES:.vala=.c)

valac_common =				\
        --quiet				\
        --ccode				\
        --vapidir $(top_srcdir)/bindings/vala	\
        --pkg grilo-0.1

vala.stamp: $(grilo_simple_playlist_VALASOURCES)
	$(VALAC) $(valac_common) $^
	touch $@

grilo_simple_playlist_CFLAGS =		\
	-DPREFIX=$(prefix)		\
	$(DEPS_CFLAGS)			\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/content

grilo_simple_playlist_LDADD = 		\
	$(DEP_LIBS)			\
	$(top_builddir)/src/lib@GRL_NAME@.la

EXTRA_DIST = $(grilo_simple_playlist_VALASOURCES) vala.stamp

MAINTAINERCLEANFILES =  \
        *.in            \
        *~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)
