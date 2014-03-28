#
# Makefile.am
#
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L. All rights reserved.

SUBDIRS = grilo-inspect grilo-launch

if BUILD_GRILO_TEST_UI
SUBDIRS += grilo-test-ui
endif

DIST_SUBDIRS = grilo-test-ui grilo-inspect grilo-launch

-include $(top_srcdir)/git.mk
