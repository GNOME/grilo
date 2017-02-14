#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

examples = [
    'browsing',
    'configuring-plugins',
    'efficient-metadata-resolution',
    'loading-plugins',
    'multivalues',
    'searching',
]

foreach e: examples
    source = e + '.c'
    executable(e,
        source,
        install: false,
        link_with: libgrl,
        dependencies: libgrl_dep)
endforeach

if enable_grlpls
    executable('browsing-pls',
        'browsing-pls.c',
        install: false,
        link_with: [libgrl, libgrlpls],
        dependencies: libgrl_dep)
endif
