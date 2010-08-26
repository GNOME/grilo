#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = grilo-inspect

if BUILD_GRILO_TEST_UI
SUBDIRS += grilo-test-ui
endif

if HAVE_VALA
SUBDIRS += vala
endif


DIST_SUBDIRS = grilo-test-ui grilo-inspect vala

MAINTAINERCLEANFILES = \
        *.in \
        *~

DISTCLEANFILES = $(MAINTAINERCLEANFILES)




