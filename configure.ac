#
# configure.ac
# 
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010, 2011 Igalia S.L.. All rights reserved.

m4_define([prj_name],    [grilo])
m4_define([prj_gir_name], [Grilo])
m4_define([grl_major_version], [0])
m4_define([grl_minor_version], [2])
m4_define([grl_micro_version], [3])
m4_define([prj_version], grl_major_version.grl_minor_version.grl_micro_version)

AC_INIT([prj_name],
        [prj_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=grilo;component=core],
        [prj_name],
        [http://live.gnome.org/Grilo])

AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src])

AM_CONFIG_HEADER(src/config.h)

AC_PROG_CC
AC_PROG_INSTALL

LT_PREREQ([2.2.6])
LT_INIT

AM_PROG_CC_C_O

AC_HEADER_STDC

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Grilo Net library

GRLNET_VERSION=0.2.2
AC_SUBST(GRLNET_VERSION)
AC_DEFINE_UNQUOTED(GRLNET_VERSION, "$GRLNET_VERSION", [Grilo Net library version])

# ----------------------------------------------------------
# LIBTOOL VERSIONING
# ----------------------------------------------------------

# Libtool library version, not to confuse with API version
# see http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html

GRL_LT_VERSION=3:0:2
GRLNET_LT_VERSION=1:1:1

AC_SUBST([GRL_LT_VERSION])
AC_SUBST([GRLNET_LT_VERSION])

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

if test "${libdir}" = '${exec_prefix}/lib'; then
   case `uname -m` in
        x86_64|ppc64|powerpc64)
                libdir='${exec_prefix}/lib64'
        ;;
        *)
                libdir='${exec_prefix}/lib'
        ;;
esac
fi

# Plugins directory

GRL_PLUGINS_DIR=`eval echo ${libdir}/${GRL_NAME}`
AC_SUBST(GRL_PLUGINS_DIR)
AC_DEFINE_UNQUOTED(GRL_PLUGINS_DIR, "$GRL_PLUGINS_DIR", [Plugins directory])

# ----------------------------------------------------------
# DEPENDENCIES
# ----------------------------------------------------------

PKG_CHECK_MODULES(DEPS, glib-2.0 >= 2.29.10 \
			gobject-2.0 \
			gmodule-2.0 \
			gio-2.0 \
			libxml-2.0)

GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS=`pkg-config --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
AC_SUBST(GLIB_MKENUMS)

# ----------------------------------------------------------
# UI Tool
# ----------------------------------------------------------

PKG_CHECK_MODULES([GTU], [ gtk+-3.0 >= 3.0 ],
                       [ HAVE_GTK=yes ],
                       [ HAVE_GTK=no ])

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

PKG_CHECK_MODULES(NET, libsoup-2.4,
			HAVE_LIBSOUP=yes,
			HAVE_LIBSOUP=no)

PKG_CHECK_EXISTS([libsoup-2.4 >= 2.33.4],
		 [HAVE_CACHED_LIBSOUP=yes],
		 [HAVE_CACHED_LIBSOUP=no])
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
AM_CONDITIONAL(BUILD_GRILO_NET_WITH_CACHE, test "x$HAVE_CACHED_LIBSOUP" = "xyes")

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

AM_CONDITIONAL(ENABLE_TESTS, test "enable_tests" = "xyes")

# ----------------------------------------------------------
# VALA BINDINGS
# ----------------------------------------------------------

AC_MSG_CHECKING([whether Vala plugin support is requested])
AC_ARG_ENABLE([vala],
        AS_HELP_STRING([--enable-vala],[Enable Vala plugin support]),
        [enable_vala=$enableval have_vala=$enableval],
        [enable_vala=autodetect have_vala=yes])
AC_MSG_RESULT([$enable_vala])
if test "x$enable_vala" != "xno"; then
	valapkg=""
	m4_foreach([VERSION], [[0.12], [0.14], [0.16], [0.18]],
		   [PKG_CHECK_EXISTS([ libvala-VERSION ],
                           [ valapkg="libvala-VERSION" ])
		   ])

	if test "x$valapkg" != "x"; then
	        PKG_CHECK_MODULES([VALA], [ ${valapkg} ],
				  [ with_vala="yes" ],
			          [ with_vala="no" ])
	else
               with_vala="no"
	fi

        if test "x$with_vala" = "xyes"; then
                AC_PATH_PROG([VALAC], [valac])
                if test "x$VALAC" = "x"; then
                        AC_MSG_ERROR([No Vala compiler found.])
                fi

                AC_PATH_PROG([VALA_GEN_INTROSPECT], [vala-gen-introspect])
                if test "x$VALA_GEN_INTROSPECT" = "x"; then
                        AC_MSG_ERROR([Cannot find "vala-gen-introspect" in your PATH])
                fi

                AC_PATH_PROG([VAPIGEN], [vapigen])
                if test "x$VAPIGEN" = "x"; then
                        AC_MSG_ERROR([Cannot fine "vapigen" in your PATH])
                fi

        elif test "x$enable_vala" = "xyes"; then
                AC_MSG_ERROR([Vala plugin support explicitly requested, but not found])
        fi
fi
AM_CONDITIONAL(HAVE_VALA, test "x$with_vala" = "xyes")

# ----------------------------------------------------------
# GTK-DOC
# ----------------------------------------------------------

GTK_DOC_CHECK([1.10],[--flavour no-tmpl])

# ----------------------------------------------------------
# GOBJECT-INTROSPECTION
# ----------------------------------------------------------

GOBJECT_INTROSPECTION_CHECK([0.9])

# ----------------------------------------------------------
# GETTEXT
# ----------------------------------------------------------

#GETTEXT_PACKAGE=grilo
#AC_SUBST(GETTEXT_PACKAGE)
#AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

#AM_GLIB_GNU_GETTEXT

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

AC_CONFIG_FILES([
  Makefile
  grilo-uninstalled.pc
  grilo-0.2.pc
  src/Makefile
  tests/Makefile
  tests/python/Makefile
  tests/python/util.py
  libs/Makefile
  libs/net/Makefile
  tools/Makefile
  tools/grilo-test-ui/Makefile
  tools/grilo-inspect/Makefile
  tools/vala/Makefile
  bindings/Makefile
  bindings/vala/Makefile
  bindings/vala/grilo-uninstalled.files
  bindings/vala/grilo-uninstalled.namespace
  bindings/vala/grilo-net-uninstalled.files
  bindings/vala/grilo-net-uninstalled.namespace
  doc/Makefile
  doc/grilo/Makefile
  examples/Makefile
])

AC_OUTPUT
