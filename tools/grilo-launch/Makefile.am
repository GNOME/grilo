#
# Makefile.am
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2014 Igalia S.L.

INCLUDES = $(DEPS_CFLAGS)

bin_PROGRAMS =       \
   grl-launch-@GRL_MAJORMINOR@

grl_launch_@GRL_MAJORMINOR@_SOURCES =  \
   grl-launch.c

grl_launch_@GRL_MAJORMINOR@_CFLAGS =   \
   -DPREFIX=$(prefix)                  \
   -I$(top_srcdir)/src                 \
   -I$(top_srcdir)/src/data

grl_launch_@GRL_MAJORMINOR@_LDADD =    \
   $(DEPS_LIBS)                        \
   $(top_builddir)/src/lib@GRL_NAME@.la

-include $(top_srcdir)/git.mk
