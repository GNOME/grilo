# meson.build
#
# Author: Victor Toso <me@victortoso.com>
#
# Copyright (C) 2018 Grilo Project

tests = [
    'autoptr',
    'lib-net',
    'media',
    'registry',
    'operations',
]

foreach t: tests
    source = t + '.c'
    exe = executable(t,
        source,
        install: false,
        link_with: [libgrl, libgrlnet],
        dependencies: [libgrl_dep, libgrlnet_dep])
    test(t, exe, timeout:10)
endforeach
