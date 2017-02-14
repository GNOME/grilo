#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

configure_file(output: 'config.h',
    configuration: cdata)

executable('grl-launch-@0@'.format(grl_majorminor),
    'grl-launch.c',
    install: true,
    link_with: libgrl,
    dependencies: [glib_dep, gobject_dep],
    include_directories: libgrl_inc)
