#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

python = find_program('python', required : false)
if not python.found()
    python = import('python3').find_python()
endif
if not python.found()
    error('Python2 or Python3 is required to compile grilo-inspect')
endif
run_command(python,
    '@0@/generate_core_keys.py'.format(meson.current_source_dir()),
    '@0@/src/grl-metadata-key.h'.format(source_root),
    '@0@/grl-core-keys.h'.format(meson.current_build_dir()))

configure_file(output: 'config.h',
    configuration: cdata)

executable('grl-inspect-@0@'.format(grl_majorminor),
    'grl-inspect.c',
    install: true,
    link_with: libgrl,
    dependencies: [glib_dep, gobject_dep],
    include_directories: libgrl_inc)
