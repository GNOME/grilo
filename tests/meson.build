# meson.build
#
# Author: Victor Toso <victortoso@gnome.org>
#
# Copyright (C) 2018 Grilo Project

tests = [
    'autoptr',
    'lib-net',
    'registry',
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
