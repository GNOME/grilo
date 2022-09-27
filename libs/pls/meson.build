#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

grlpls_sources = [
    'grl-pls.c',
]

grlpls_headers = [
    'grl-pls.h',
]

configure_file(output: 'config.h',
    configuration: cdata)

install_headers(grlpls_headers,
    subdir: '@0@/pls'.format(grl_name))

libgrlpls_inc = include_directories(['..'])

libgrlpls = shared_library('grlpls-@0@'.format(grl_majorminor),
    sources: grlpls_sources,
    install: true,
    soversion: soversion,
    version: grlpls_lt_version,
    c_args: '-DHAVE_CONFIG_H',
    dependencies: [totem_plparser_dep, libgrl_dep],
    include_directories: libgrl_inc)

libgrlpls_dep = declare_dependency(link_with: libgrlpls,
    include_directories: [libgrl_inc, libgrlpls_inc],
    dependencies: [totem_plparser_dep, libgrl_dep])

grlpls_pc = pkgconfig.generate(libgrlpls,
    filebase: 'grilo-pls-@0@'.format(grl_majorminor),
    requires: libgrl,
    subdirs: grl_name,
    name: 'Grilo playlist library',
    description: 'Grilo playlist utility',
    version: grlpls_version,
    variables: [
        'girdir=' + girdir_for_pc_file,
        'typelibdir=' + typelibdir_for_pc_file,
    ],
)

if enable_gir
    gnome.generate_gir(libgrlpls,
        sources: grlpls_headers + grlpls_sources,
        namespace: 'GrlPls',
        nsversion: grl_majorminor,
        identifier_prefix: 'GrlPls',
        symbol_prefix: 'grl_pls',
        dependencies: [ gobject_dep, gio_dep ],
        includes: [ 'GObject-2.0', 'Gio-2.0', grl_gir[0] ],
        include_directories: libs_inc,
        install: true,
        extra_args: [ '--c-include=pls/grl-pls.h' ])
endif
