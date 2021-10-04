# meson.build
#
# Author: Victor Toso <me@victortoso.com>
#
# Copyright (C) 2018 Grilo Project

tests = [
    'autoptr',
    'media',
    'registry',
    'operations',
]

test_link_with = [libgrl]
test_deps = [libgrl_dep]

if enable_grlnet
    tests += ['lib-net']
    test_link_with += libgrlnet
    test_deps += libgrlnet_dep
endif

foreach t: tests
    source = t + '.c'
    exe = executable(t,
        source,
        install: false,
        link_with: test_link_with,
        dependencies: test_deps)
    test(t, exe, timeout:10)
endforeach
