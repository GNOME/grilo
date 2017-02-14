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

libgrlpls = shared_library('grlpls-@0@'.format(grl_majorminor),
    sources: grlpls_sources,
    install: true,
    soversion: soversion,
    version: grlpls_lt_version,
    c_args: '-DHAVE_CONFIG_H',
    dependencies: [totem_plparser_dep, libgrl_dep],
    include_directories: libgrl_inc)

if enable_gir
    gnome.generate_gir(libgrlpls,
        sources: grlpls_headers,
        namespace: 'GrlPls',
        nsversion: grl_majorminor,
        identifier_prefix: 'GrlPls',
        symbol_prefix: 'grl_pls',
        includes: [ 'GObject-2.0', 'Gio-2.0' ],
        install: true)
endif
