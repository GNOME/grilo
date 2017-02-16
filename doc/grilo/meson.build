#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

types = configure_file(input : 'grilo.types',
  output : 'grilo.types',
  configuration : configuration_data())

gnome.gtkdoc('grilo',
    main_sgml: 'grilo-docs.sgml',
    src_dir: [
        join_paths(meson.source_root(), 'src'),
        join_paths(meson.source_root(), 'libs'),
    ],
    dependencies: libgrl_dep,
    gobject_typesfile: types,
    install: true,
    content_files: [
        'environment-setup.xml',
        'plugins-sources.xml',
        'plugins-testing.xml',
        'quick-start.xml',
        'writing-apps.xml',
        'overview.xml',
        join_paths(meson.source_root(), 'examples', 'configuring-plugins.c'),
        join_paths(meson.source_root(), 'examples', 'browsing.c'),
        join_paths(meson.source_root(), 'examples', 'searching.c'),
        join_paths(meson.source_root(), 'examples', 'multivalues.c'),
        join_paths(meson.source_root(), 'examples', 'efficient-metadata-resolution.c'),
    ]
)
