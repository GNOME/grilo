#
# configure.ac
# 
# Author: Iago Toral Quiroga <itoral@igalia.com>
#
# Copyright (C) 2010, 2011 Igalia S.L.. All rights reserved.

m4_define([prj_name],    [grilo])
m4_define([prj_gir_name], [Grilo])
m4_define([grl_major_version], [0])
m4_define([grl_minor_version], [1])
m4_define([grl_micro_version], [9])
m4_define([prj_version], grl_major_version.grl_minor_version.grl_micro_version)

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

GRL_MAJORMINOR=grl_major_version.grl_minor_version
GRL_MAJORMINOR_NORM=grl_major_version[_]grl_minor_version
AC_SUBST(GRL_MAJORMINOR)
AC_SUBST(GRL_MAJORMINOR_NORM)
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
# DEPENDENCIES
# ----------------------------------------------------------

PKG_CHECK_MODULES(DEPS, glib-2.0 >= 2.22 \
			gobject-2.0 \
			gmodule-2.0 \
			gio-2.0 \
			libxml-2.0)

GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS=`pkg-config --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
AC_SUBST(GLIB_MKENUMS)

# Tools

PKG_CHECK_MODULES(GTU, gtk+-2.0 \
                       gconf-2.0,
                       BUILD_GRILO_TEST_UI=yes,
                       BUILD_GRILO_TEST_UI=no)

AM_CONDITIONAL(BUILD_GRILO_TEST_UI, test "x$BUILD_GRILO_TEST_UI" = "xyes")

# ----------------------------------------------------------
# NET LIBRARY
# ----------------------------------------------------------

PKG_CHECK_MODULES(NET, libsoup-2.4,
			HAVE_LIBSOUP=yes,
			HAVE_LIBSOUP=no)

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

VALA_REQS=0.7.2

AC_MSG_CHECKING([whether Vala plugin support is requested])
AC_ARG_ENABLE([vala],
        AS_HELP_STRING([--enable-vala],[Enable Vala plugin support]),
        [enable_vala=$enableval have_vala=$enableval],
        [enable_vala=autodetect have_vala=yes])
AC_MSG_RESULT([$enable_vala])
if test "x$enable_vala" != "xno"; then
        PKG_CHECK_EXISTS([ libvala-0.12 ],
                         [ valapkg="libvala-0.12" ],
                         [ PKG_CHECK_EXISTS([ vala-0.10 ],
                                            [ valapkg="vala-0.10" ],
                                            [ PKG_CHECK_EXISTS([ vala-1.0 >= $VALA_REQS ],
                                                               [ valapkg="vala-1.0" ],
                                                               [ valapkg="" ] )
                                             ] )
                         ] )

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

                VAPIDIR=`$PKG_CONFIG --variable=vapidir $valapkg`
                AC_SUBST([VAPIDIR])

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

GOBJECT_INTROSPECTION_CHECK([0.6.7])

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

AC_CONFIG_FILES([
  Makefile
  grilo-uninstalled.pc
  grilo-0.1.pc
  grilo-net-uninstalled.pc
  grilo-net-0.1.pc
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
  doc/libs/Makefile
])

AC_OUTPUT
