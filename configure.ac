#
# configure.ac
# 
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010 Igalia S.L.. All rights reserved.

m4_define([prj_name],    [grilo])
m4_define([prj_version], [0.1.5])
m4_define([prj_gir_name], [Grilo])

AC_INIT([prj_name], [prj_version])

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

# ----------------------------------------------------------
# ENVIRONMENT CONFIGURATION
# ----------------------------------------------------------

GNOME_COMPILE_WARNINGS([maximum])
CFLAGS+=" ${WARN_CFLAGS} -Wmissing-declarations -std=c99 "

GNOME_MAINTAINER_MODE_DEFINES

# ----------------------------------------------------------
# DEFINITIONS
# ----------------------------------------------------------

# ABI version

GRL_MAJORMINOR=0.1
AC_SUBST(GRL_MAJORMINOR)
AC_DEFINE_UNQUOTED(GRL_MAJORMINOR, "$GRL_MAJORMINOR", [library major.minor version])

# Framework name (contains ABI version)

GRL_NAME="${PACKAGE_NAME}-${GRL_MAJORMINOR}"
AC_SUBST(GRL_NAME)
AC_DEFINE_UNQUOTED(GRL_NAME, "$GRL_NAME", [framework name])

# Give parameters a default value

if test $prefix = "NONE" ; then
	prefix="/usr/local"
fi

if test $libdir = "\${exec_prefix}/lib" ; then
	libdir="$prefix/lib"
fi

if test $datadir = "NONE" ; then
	datadir="$prefix/share"
fi

# Plugins directory

GRL_PLUGINS_DIR="${libdir}/${GRL_NAME}"
AC_SUBST(GRL_PLUGINS_DIR)
AC_DEFINE_UNQUOTED(GRL_PLUGINS_DIR, "$GRL_PLUGINS_DIR", [Plugins directory])

# Plugins configuration directory

GRL_PLUGINS_CONF_DIR="${datadir}/${GRL_NAME}/plugins"
AC_SUBST(GRL_PLUGINS_CONF_DIR)

# ----------------------------------------------------------
# UNINSTALLED SUPPORT
# ----------------------------------------------------------

AC_ARG_ENABLE(uninstalled,
        [  --enable-uninstalled run uninstalled],,
        enable_uninstalled=no)
if test "x$enable_uninstalled" = "xyes"; then
        AC_MSG_WARN("Configuring for running uninstalled!")
fi

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
# VALA BINDINGS
# ----------------------------------------------------------

VALA_REQS=0.7.2

AC_MSG_CHECKING([whether Vala plugin support is requested])
AC_ARG_ENABLE([vala],
        AS_HELP_STRING([--enable-vala],[Enable Vala plugin support]),
        [enable_vala=$enableval have_vala=$enableval],
        [enable_vala=autodetect have_vala=yes])
AC_MSG_RESULT([$enable_vala])
if test "x$enable_vala" != "xno"; then
        PKG_CHECK_MODULES(VALA, vala-1.0 >= $VALA_REQS, with_vala=yes,
                          with_vala=no)
        if test "x$with_vala" = "xyes"; then
                VALAC="`pkg-config --variable=prefix vala-1.0`/bin/valac"
                AC_SUBST([VALAC])
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

GOBJECT_INTROSPECTION_CHECK([0.6.7])

# ----------------------------------------------------------
# GETTEXT
# ----------------------------------------------------------

#GETTEXT_PACKAGE=grilo
#AC_SUBST(GETTEXT_PACKAGE)
#AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

#AM_GLIB_GNU_GETTEXT

# ----------------------------------------------------------
# DEPENDENCIES
# ----------------------------------------------------------

PKG_CHECK_MODULES(DEPS, glib-2.0 \
			gobject-2.0 \
			gmodule-2.0 \
                        gio-2.0 \
			libxml-2.0)

# Tools

PKG_CHECK_MODULES(GTU, gtk+-2.0 \
                       gconf-2.0,
                       BUILD_GRILO_TEST_UI=yes,
                       BUILD_GRILO_TEST_UI=no)

AM_CONDITIONAL(BUILD_GRILO_TEST_UI, test "x$BUILD_GRILO_TEST_UI" = "xyes")

# ----------------------------------------------------------
# WORKAROUNDS
# ----------------------------------------------------------

AC_SUBST([abs_top_builddir])
AC_SUBST([builddir])

# ----------------------------------------------------------
# OUTPUT
# ----------------------------------------------------------

AC_CONFIG_FILES([
  Makefile
  grilo-0.1-uninstalled.pc
  grilo-0.1.pc
  src/Makefile
  src/tests/Makefile
  tools/Makefile
  tools/grilo-test-ui/Makefile
  tools/vala/Makefile
  bindings/Makefile
  bindings/vala/Makefile
  doc/Makefile
  doc/reference/Makefile
])

AC_OUTPUT
