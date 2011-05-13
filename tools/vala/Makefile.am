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
        --pkg grilo-0.2

vala.stamp: $(grilo_simple_playlist_VALASOURCES)
	$(VALA_V)$(VALAC) $(valac_common) $^
	touch $@

grilo_simple_playlist_CFLAGS =		\
	-DPREFIX=$(prefix)		\
	$(DEPS_CFLAGS)			\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/src/data

grilo_simple_playlist_LDADD = 		\
	$(DEPS_LIBS)			\
	$(top_builddir)/src/lib@GRL_NAME@.la

EXTRA_DIST = $(grilo_simple_playlist_VALASOURCES) vala.stamp

MAINTAINERCLEANFILES =  \
        *.in            \
        *~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)

VALA_V = $(VALA_V_$(V))
VALA_V_ = $(VALA_V_$(AM_DEFAULT_VERBOSITY))
VALA_V_0 = @echo "  VALAC " $^;
