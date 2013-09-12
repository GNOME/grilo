#
# Makefile.am
#
# Author: Víctor M. Jáquez L. <vjaquez@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS =

if BUILD_GRILO_NET
SUBDIRS += net
endif

if BUILD_GRILO_PLS
SUBDIRS += pls
endif

DIST_SUBDIRS = net pls

-include $(top_srcdir)/git.mk
