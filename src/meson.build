#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

grl_marshal = gnome.genmarshal('grl-marshal',
    sources: 'grl-marshal.list',
    prefix: 'grl_marshal')

grl_type_builtins = gnome.mkenums('grl-type-builtins',
    sources: [
        'data/grl-media.h',
        'grl-caps.h',
        'grl-metadata-key.h',
        'grl-operation-options.h',
        'grl-source.h',
    ],
    c_template: 'grl-type-builtins.c.template',
    h_template: 'grl-type-builtins.h.template')

grl_sources = [
    'data/grl-config.c',
    'data/grl-data.c',
    'data/grl-media.c',
    'data/grl-related-keys.c',
    'grilo.c',
    'grl-caps.c',
    'grl-log.c',
    'grl-metadata-key.c',
    'grl-multiple.c',
    'grl-operation-options.c',
    'grl-operation.c',
    'grl-plugin.c',
    'grl-range-value.c',
    'grl-registry.c',
    'grl-source.c',
    'grl-sync.c',
    'grl-util.c',
    'grl-value-helper.c',
]

grl_headers = [
    'data/grl-config.h',
    'data/grl-data.h',
    'data/grl-media.h',
    'data/grl-related-keys.h',
    'grilo.h',
    'grl-caps.h',
    'grl-definitions.h',
    'grl-error.h',
    'grl-log.h',
    'grl-metadata-key.h',
    'grl-multiple.h',
    'grl-operation-options.h',
    'grl-operation.h',
    'grl-plugin.h',
    'grl-range-value.h',
    'grl-registry.h',
    'grl-source.h',
    'grl-util.h',
    'grl-value-helper.h',
]

grl_priv_headers = [
    'grl-metadata-key-priv.h',
    'grl-operation-options-priv.h',
    'grl-operation-priv.h',
    'grl-plugin-priv.h',
    'grl-registry-priv.h',
    'grl-sync-priv.h',
]

configure_file(output: 'config.h',
    configuration: cdata)

install_headers(grl_headers,
    subdir: grl_name)

libgrl_inc = include_directories([
    '.',
    'data',
])

c_args = [
    '-DGRILO_COMPILATION',
    '-DG_LOG_DOMAIN="Grilo"',
    '-DHAVE_CONFIG_H',
    '-DGRL_NAME="@0@"'.format(grl_name)
]

libgrl_dep_libs = []
libgrl_deps = [
    glib_dep, gobject_dep, gmodule_dep, gio_dep
]

if libxml_dep.type_name() == 'pkgconfig'
  libgrl_deps += libxml_dep
elif libxml_dep.type_name() == 'library'
  libgrl_dep_libs += libxml_dep
endif

libgrl_link_args = []
if cc.get_argument_syntax() != 'msvc'
  libgrl_link_args += '-Wl,--no-undefined'
endif

libgrl = shared_library(grl_name,
    sources: grl_sources + grl_marshal + grl_type_builtins + grl_priv_headers,
    install: true,
    soversion: soversion,
    version: grilo_lt_version,
    dependencies: libgrl_deps,
    c_args: c_args,
    link_args: libgrl_link_args,
    include_directories: libgrl_inc)

libgrl_dep = declare_dependency(link_with: libgrl,
    include_directories: libgrl_inc,
    dependencies: libgrl_deps)

grl_pc = pkgconfig.generate(libgrl,
    filebase: grl_name,
    requires: libgrl_deps,
    libraries: libgrl_dep_libs,
    subdirs: grl_name,
    name: 'Grilo Framework',
    description: 'Grilo Framework for multimedia developers',
    version: grilo_version,
    variables: [
        'exec_prefix=${prefix}',
        'datarootdir=' + datadir_for_pc_file,
        'datadir=${datarootdir}',
        'girdir=' + girdir_for_pc_file,
        'typelibdir=' + typelibdir_for_pc_file,
        'plugindir=' + plugins_dir_for_pc_file,
    ],
)

if enable_gir
    gir_extra_args = [
        '--c-include=grilo.h',
        '--cflags-begin',
    ] + c_args + [
        '--cflags-end',
    ]

    grl_gir = gnome.generate_gir(libgrl,
        sources: grl_headers + grl_sources + grl_type_builtins,
        namespace: 'Grl',
        nsversion: grl_majorminor,
        identifier_prefix: 'Grl',
        symbol_prefix: 'grl',
        dependencies: [ gobject_dep, gmodule_dep, gio_dep ],
        includes: [ 'GObject-2.0', 'GModule-2.0', 'Gio-2.0' ],
        install: true,
        install_dir_gir: girdir,
        install_dir_typelib: typelibdir,
        extra_args: gir_extra_args)
endif
