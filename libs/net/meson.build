#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

grlnet_sources = [
    'grl-net-mock.c',
    'grl-net-wc.c',
]

grlnet_headers = [
    'grl-net.h',
    'grl-net-wc.h',
]

grlnet_priv_headers = [
    'grl-net-mock-private.h',
]

configure_file(output: 'config.h',
    configuration: cdata)

install_headers(grlnet_headers,
    subdir: '@0@/net'.format(grl_name))

libgrlnet = shared_library('grlnet-@0@'.format(grl_majorminor),
    sources: grlnet_sources + grlnet_priv_headers,
    soversion: soversion,
    version: grlnet_lt_version,
    install: true,
    c_args: '-DHAVE_CONFIG_H',
    dependencies: [libsoup_dep, libgrl_dep],
    include_directories: libgrl_inc)

if enable_gir
    gir_extra_args = [
        '--c-include=net/grl-net.h',
        '--cflags-begin',
        '-I' + '@0@/libs'.format(meson.source_root()),
        '--cflags-end',
    ]

    grlnet_gir = gnome.generate_gir(libgrlnet,
        sources: grlnet_headers,
        namespace: 'GrlNet',
        nsversion: grl_majorminor,
        identifier_prefix: 'GrlNet',
        symbol_prefix: 'grl_net',
        includes: [ 'GObject-2.0', 'Gio-2.0', 'Soup-2.4' ],
        install: true,
        extra_args: gir_extra_args)
endif
