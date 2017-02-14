#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

configure_file(output: 'config.h',
    configuration: cdata)

sources = [
    'main.c',
]

if oauth_dep.found()
    sources += [
        'flickr-oauth.c',
        'flickr-oauth.h',
    ]
endif

executable('grilo-test-ui-@0@'.format(grl_majorminor),
    sources,
    install: true,
    link_with: libgrl,
    dependencies: [glib_dep, gobject_dep, gtk_dep, oauth_dep],
    include_directories: libgrl_inc)
