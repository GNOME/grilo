#
# configure.ac
# 
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010, 2011 Igalia S.L.. All rights reserved.

m4_define([prj_name],    [grilo])
m4_define([prj_gir_name], [Grilo])
m4_define([grl_major_version], [0])
m4_define([grl_minor_version], [3])
m4_define([grl_micro_version], [0])
m4_define([prj_version], grl_major_version.grl_minor_version.grl_micro_version)

AC_INIT([prj_name],
        [prj_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=grilo;component=core],
        [prj_name],
        [https://wiki.gnome.org/Projects/Grilo])

AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz no-dist-gzip])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/grilo.h])

AC_CONFIG_HEADERS([config.h])

AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL

LT_PREREQ([2.2.6])
LT_INIT

AM_PROG_CC_C_O

AC_HEADER_STDC

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Grilo Net library

GRLNET_VERSION=0.2.5
AC_SUBST(GRLNET_VERSION)
AC_DEFINE_UNQUOTED(GRLNET_VERSION, "$GRLNET_VERSION", [Grilo Net library version])

# Grilo Pls library

GRLPLS_VERSION=0.2.2
AC_SUBST(GRLPLS_VERSION)
AC_DEFINE_UNQUOTED(GRLPLS_VERSION, "$GRLPLS_VERSION", [Grilo Pls library version])

# ----------------------------------------------------------
# LIBTOOL VERSIONING
# ----------------------------------------------------------

# Libtool library version, not to confuse with API version
# see http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html

GRL_LT_VERSION=10:0:0
GRLNET_LT_VERSION=1:9:1
GRLPLS_LT_VERSION=0:4:0

AC_SUBST([GRL_LT_VERSION])
AC_SUBST([GRLNET_LT_VERSION])
AC_SUBST([GRLPLS_LT_VERSION])

# ----------------------------------------------------------
# ENVIRONMENT CONFIGURATION
# ----------------------------------------------------------

GNOME_COMPILE_WARNINGS([maximum])
CFLAGS+=" ${WARN_CFLAGS} -Wmissing-declarations -std=c99 "

GNOME_MAINTAINER_MODE_DEFINES

# ----------------------------------------------------------
# DEFINITIONS
# ----------------------------------------------------------

# API version

GRL_MAJORMINOR=grl_major_version.grl_minor_version
GRL_MAJORMINOR_NORM=grl_major_version[_]grl_minor_version
AC_SUBST(GRL_MAJORMINOR)
AC_SUBST(GRL_MAJORMINOR_NORM)
AC_DEFINE_UNQUOTED(GRL_MAJORMINOR, "$GRL_MAJORMINOR", [library major.minor version])

# Framework name (contains API version)

GRL_NAME="${PACKAGE_NAME}-${GRL_MAJORMINOR}"
AC_SUBST(GRL_NAME)
AC_DEFINE_UNQUOTED(GRL_NAME, "$GRL_NAME", [framework name])

# Give parameters a default value

if test "x${prefix}" = "xNONE" ; then
   prefix="${ac_default_prefix}"
fi

if test "x${exec_prefix}" = "xNONE" ; then
   exec_prefix="${prefix}"
fi

# Plugins directory

GRL_PLUGINS_DIR=`eval echo ${libdir}/${GRL_NAME}`
AC_SUBST(GRL_PLUGINS_DIR)
AC_DEFINE_UNQUOTED(GRL_PLUGINS_DIR, "$GRL_PLUGINS_DIR", [Plugins directory])

# ----------------------------------------------------------
# DEPENDENCIES
# ----------------------------------------------------------

PKG_CHECK_MODULES(DEPS, glib-2.0 >= 2.34 \
			gobject-2.0 \
			gmodule-2.0 \
			gio-2.0 \
			libxml-2.0)


GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS=`pkg-config --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
AC_SUBST(GLIB_MKENUMS)

PKG_CHECK_MODULES(GIO_WITH_NM, gio-2.0 >= 2.43.1, [has_gio_with_netmon=yes], [has_gio_with_netmon=false])
if test "x$has_gio_with_netmon" = "xyes"; then
    AC_DEFINE(HAVE_GIO_WITH_NETMON, 1, [GNetworkConnectivityMonitor available])
fi

# ----------------------------------------------------------
# UI Tool
# ----------------------------------------------------------

PKG_CHECK_MODULES([GTU], [ gtk+-3.0 >= 3.0 ],
                       [ HAVE_GTK=yes ],
                       [ HAVE_GTK=no ])

PKG_CHECK_MODULES(OAUTH, oauth, [HAVE_OAUTH=yes], [HAVE_OAUTH=no])

AM_CONDITIONAL(HAVE_OAUTH, test "x$HAVE_OAUTH" = "xyes")

if test "x$HAVE_OAUTH" = "xyes"; then
   AC_DEFINE(HAVE_OAUTH, 1, [OAuth available])
else
   AC_MSG_WARN([oauth missing, unable to configure Flickr personal accounts])
fi

AC_ARG_ENABLE(test-ui,
        AS_HELP_STRING([--enable-test-ui],
                [Build Test UI (default: auto)]),
        [
                case "$enableval" in
                     yes)
                        if test "x$HAVE_GTK" = "xno"; then
                           AC_MSG_ERROR([gtk+-3.0 >= 3.0 not found, install it or use --disable-test-ui])
                        fi
                        ;;
                     no)
                        HAVE_GTK=no
                        ;;
                esac
        ])

AM_CONDITIONAL(BUILD_GRILO_TEST_UI, test "x$HAVE_GTK" = "xyes")

# ----------------------------------------------------------
# NET LIBRARY
# ----------------------------------------------------------

PKG_CHECK_MODULES(NET, libsoup-2.4 >= 2.33.4,
			HAVE_LIBSOUP=yes,
			HAVE_LIBSOUP=no)

PKG_CHECK_EXISTS([libsoup-2.4 >= 2.41.3],
                 [HAVE_LIBSOUP_REQUESTER_DEPRECATED=yes],
                 [HAVE_LIBSOUP_REQUESTER_DEPRECATED=no])

AC_ARG_ENABLE([grl_net],
        AS_HELP_STRING([--enable-grl-net],
                [Enable Grilo Net library (default: auto)]),
        [
                case "$enableval" in
                     yes)
                        if test "x$HAVE_LIBSOUP" = "xno"; then
                           AC_MSG_ERROR([libsoup-2.4 not found, install it or use --disable-grl-net])
                        fi
                        ;;
                     no)
                        HAVE_LIBSOUP=no
                        ;;
                esac
        ])

AM_CONDITIONAL(BUILD_GRILO_NET, test "x$HAVE_LIBSOUP" = "xyes")
AM_CONDITIONAL(BUILD_GRILO_NET_WITH_DEPRECATED_REQUESTER, test "x$HAVE_LIBSOUP_REQUESTER_DEPRECATED" = "xyes")

# ----------------------------------------------------------
# PLS LIBRARY
# ----------------------------------------------------------

PKG_CHECK_MODULES(TOTEM_PL_PARSER, totem-plparser >= 3.4.1, HAVE_TOTEM_PL_PARSER=yes, HAVE_TOTEM_PL_PARSER=no)

AC_ARG_ENABLE([grl_pls],
        AS_HELP_STRING([--enable-grl-pls],
                [Enable Grilo Pls library (default: auto)]),
        [
                case "$enableval" in
                     yes)
                        if test "x$HAVE_TOTEM_PL_PARSER" = "xno"; then
                           AC_MSG_ERROR([totem-pl-parser not found, install it or use --disable-grl-pls])
                        fi
                        ;;
                     no)
                        HAVE_TOTEM_PL_PARSER=no
                        ;;
                esac
        ])

AM_CONDITIONAL(BUILD_GRILO_PLS, test "x$HAVE_TOTEM_PL_PARSER" = "xyes")

# ----------------------------------------------------------
# DEBUG SUPPORT
# ----------------------------------------------------------

AC_ARG_ENABLE(debug,
        [  --enable-debug include debug symbols],,
        enable_debug=yes)
if test "x$enable_debug" = "xyes"; then
        AC_MSG_WARN("Configuring with debugging options enabled!")
	CFLAGS="$CFLAGS -g3 -O0"
fi

# ----------------------------------------------------------
# TESTS SUPPORT
# ----------------------------------------------------------

AC_ARG_ENABLE([tests],
        AS_HELP_STRING([--enable-tests],[build unit tests]),
        enable_tests=yes)

AM_CONDITIONAL(ENABLE_TESTS, test "x$enable_tests" = "xyes")

# ----------------------------------------------------------
# GTK-DOC
# ----------------------------------------------------------

GTK_DOC_CHECK([1.10],[--flavour no-tmpl])

# ----------------------------------------------------------
# GOBJECT-INTROSPECTION
# ----------------------------------------------------------

GOBJECT_INTROSPECTION_CHECK([0.9])

# ----------------------------------------------------------
# VALA BINDINGS
# ----------------------------------------------------------

VAPIGEN_CHECK([0.27])

# ----------------------------------------------------------
# GETTEXT/INTLTOOL
# ----------------------------------------------------------

IT_PROG_INTLTOOL([0.40.0])
GETTEXT_PACKAGE=grilo
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The domain to use with gettext])

# ----------------------------------------------------------
# WORKAROUNDS
# ----------------------------------------------------------

AC_SUBST([abs_top_builddir])
AC_SUBST([builddir])

# ----------------------------------------------------------
# OUTPUT
# ----------------------------------------------------------

if test "x$HAVE_LIBSOUP" = "xyes"; then
   AC_CONFIG_FILES([grilo-net-uninstalled.pc grilo-net-0.2.pc])
fi

if test "x$HAVE_TOTEM_PL_PARSER" = "xyes"; then
   AC_CONFIG_FILES([grilo-pls-uninstalled.pc grilo-pls-0.2.pc])
fi

AC_CONFIG_FILES([
  Makefile
  grilo-uninstalled.pc
  grilo-0.3.pc
  src/Makefile
  src/data/Makefile
  po/Makefile.in
  po/Makefile
  tests/Makefile
  tests/python/Makefile
  tests/python/util.py
  libs/Makefile
  libs/net/Makefile
  libs/pls/Makefile
  tools/Makefile
  tools/grilo-test-ui/Makefile
  tools/grilo-inspect/Makefile
  tools/grilo-launch/Makefile
  bindings/Makefile
  bindings/vala/Makefile
  doc/Makefile
  doc/grilo/Makefile
  examples/Makefile
])

AC_OUTPUT
