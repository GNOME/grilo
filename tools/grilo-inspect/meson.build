#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

python = import('python').find_installation('python3')

run_command(python.full_path(),
    '@0@/generate_core_keys.py'.format(meson.current_source_dir()),
    '@0@/src/grl-metadata-key.h'.format(source_root),
    '@0@/grl-core-keys.h'.format(meson.current_build_dir()),
    check: true)

configure_file(output: 'config.h',
    configuration: cdata)

executable('grl-inspect-@0@'.format(grl_majorminor),
    'grl-inspect.c',
    install: true,
    link_with: libgrl,
    dependencies: [glib_dep, gobject_dep],
    include_directories: libgrl_inc)
