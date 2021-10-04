#
# meson.build
#
# Author: Juan A. Suarez Romero <jasuarez@igalia.com>
#
# Copyright (C) 2016 Igalia S.L. All rights reserved.

vala_sources = [ # LIBRARY, GIR, DEPS
    ['grilo-@0@'.format(grl_majorminor), grl_gir[0], ['gmodule-2.0', 'gio-2.0']],
]

if enable_grlnet
    vala_sources += [['grilo-net-@0@'.format(grl_majorminor), grlnet_gir[0], ['gio-2.0']]]
endif

foreach s: vala_sources
    custom_vala = '@0@-custom.vala'.format(s[0])
    lib = s[0]
    gir = s[1]
    deps = s[2]

    gnome.generate_vapi(lib,
        sources: [custom_vala, gir],
        packages: deps,
        install: true,
        install_dir: vapidir)
endforeach

