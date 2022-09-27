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

libgrlnet_inc = include_directories([
    '..',
])


libgrlnet = shared_library('grlnet-@0@'.format(grl_majorminor),
    sources: grlnet_sources + grlnet_priv_headers,
    soversion: soversion,
    version: grlnet_lt_version,
    install: true,
    c_args: '-DHAVE_CONFIG_H',
    dependencies: [libsoup_dep, libgrl_dep],
    include_directories: [libgrl_inc, libgrlnet_inc])

libgrlnet_dep = declare_dependency(link_with: libgrlnet,
    include_directories: [libgrl_inc, libgrlnet_inc],
    dependencies: [libsoup_dep, libgrl_dep],
    variables: {'soupapiversion': soup_api_version})

grlnet_pc = pkgconfig.generate(libgrlnet,
    filebase: 'grilo-net-@0@'.format(grl_majorminor),
    requires: libgrl,
    subdirs: grl_name,
    name: 'Grilo networking library',
    description: 'Grilo networking helpers for plug-ins',
    version: grlnet_version,
    variables: [
        'girdir=' + girdir_for_pc_file,
        'typelibdir=' + typelibdir_for_pc_file,
        'soupapiversion=' + soup_api_version,
    ],
)

if enable_gir
    grlnet_gir = gnome.generate_gir(libgrlnet,
        sources: grlnet_headers + grlnet_sources,
        namespace: 'GrlNet',
        nsversion: grl_majorminor,
        identifier_prefix: 'GrlNet',
        symbol_prefix: 'grl_net',
        dependencies: [ gobject_dep, gio_dep, libsoup_dep ],
        includes: [ 'GObject-2.0', 'Gio-2.0', 'Soup-' + soup_api_version ],
        include_directories: libs_inc,
        install: true,
        extra_args: [ '--c-include=net/grl-net.h' ])
endif
