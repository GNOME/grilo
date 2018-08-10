#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

project('grilo', 'c',
        version: '0.3.7',
        default_options: [
            'buildtype=debugoptimized',
            'c_std=gnu99',
            'warning_level=1'
        ],
        license: 'LGPL 2.1',
        meson_version: '>= 0.37.0')

grilo_version = meson.project_version()
grlnet_version = meson.project_version()
grlpls_version = meson.project_version()

# maintaining compatibility with the previous libtool versioning
# current = minor * 100 + micro
version_array = grilo_version.split('.')
soversion = version_array[0]
current = version_array[1].to_int() * 100 + version_array[2].to_int()

# Increase the age everytime new API is added
grilo_interface_age = 1
grlnet_interface_age = 0
grlpls_interface_age = 0

grilo_lt_version = '@0@.@1@.@2@'.format(soversion, current, grilo_interface_age)
grlnet_lt_version = '@0@.@1@.@2@'.format(soversion, current, grlnet_interface_age)
grlpls_lt_version = '@0@.@1@.@2@'.format(soversion, current, grlpls_interface_age)

grl_majorminor = '@0@.@1@'.format(version_array[0], version_array[1])
grl_name = '@0@-@1@'.format(meson.project_name(), grl_majorminor)

glib2_required = '2.44'
glib2_required_info = '>= @0@'.format(glib2_required)

gio_dep = dependency('gio-2.0', version: glib2_required_info, required: true)
glib_dep = dependency('glib-2.0', version: glib2_required_info, required: true)
gmodule_dep = dependency('gmodule-2.0', version: glib2_required_info, required: true)
gobject_dep = dependency('gobject-2.0', version: glib2_required_info, required: true)
libxml_dep = dependency('libxml-2.0', required: true)

enable_grlnet = get_option('enable-grl-net')
if enable_grlnet
    libsoup_dep = dependency('libsoup-2.4', version: '>= 2.41.3', required: true)
endif

enable_grlpls = get_option('enable-grl-pls')
if enable_grlpls
    totem_plparser_dep = dependency('totem-plparser', version: '>= 3.4.1', required: true)
endif

enable_gir = get_option('enable-introspection') or get_option('enable-vala')
if enable_gir
   find_program('g-ir-scanner', required: true)
endif

enable_vala = get_option('enable-vala')
if enable_vala
    vapigen = find_program('vapigen', required: true)
endif

enable_testui = get_option('enable-test-ui')
if enable_testui
    gtk_dep = dependency('gtk+-3.0', version: '>= 3.0', required: true)
    oauth_dep = dependency('oauth', required: false)
endif

prefix = get_option('prefix')
libdir = join_paths(prefix, get_option('libdir'))
plugins_dir = join_paths(libdir, grl_name)
localedir = join_paths(prefix, get_option('localedir'))

cdata = configuration_data()
cdata.set_quoted('VERSION', grilo_version)
cdata.set('GRLNET_VERSION', grlnet_version)
cdata.set('GRLPLS_VERSION', grlpls_version)
cdata.set_quoted('GRL_MAJORMINOR', grl_majorminor)
cdata.set_quoted('GETTEXT_PACKAGE', meson.project_name())
cdata.set_quoted('GRL_PLUGINS_DIR', plugins_dir)
cdata.set_quoted('LOCALEDIR', localedir)
if enable_testui
   cdata.set('HAVE_OAUTH', oauth_dep.found())
endif

pkgconfig_files = [
    'grilo-@0@'.format(grl_majorminor),
    'grilo-net-@0@'.format(grl_majorminor),
    'grilo-pls-@0@'.format(grl_majorminor),
]

pkgconf = configuration_data()
pkgconf.set('prefix', prefix)
pkgconf.set('exec_prefix', '${prefix}')
pkgconf.set('libdir', libdir)
pkgconf.set('includedir', '${prefix}/@0@'.format(get_option('includedir')))
pkgconf.set('GRL_NAME', grl_name)
pkgconf.set('GRL_PLUGINS_DIR', plugins_dir)
pkgconf.set('INTROSPECTION_GIRDIR', '${datarootdir}/gir-1.0')
pkgconf.set('INTROSPECTION_TYPELIBDIR', '${libdir}/girepository-1.0'.format(get_option('libdir')))
pkgconf.set('VERSION', grilo_version)
pkgconf.set('GRLNET_VERSION', grlnet_version)
pkgconf.set('GRLPLS_VERSION', grlpls_version)

foreach p: pkgconfig_files
    infile = p + '.pc.in'
    outfile = p + '.pc'
    configure_file(input: infile,
        output: outfile,
        configuration: pkgconf,
        install_dir: '@0@/pkgconfig'.format(get_option('libdir')))
endforeach

gnome = import('gnome')

subdir('src')
subdir('libs')
subdir('bindings')
subdir('po')
subdir('tools')
subdir('examples')
subdir('doc')
