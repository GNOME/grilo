#!/usr/bin/python3

import gi
gi.require_version('GrlNet', '0.3')
from gi.repository import GrlNet
from gi.repository import GLib

def check_result(wc, res, loop):
    loop.quit()
    [success, contents] = wc.request_finish(res)
    assert(success == True)

def _init():
    wc = GrlNet.Wc.new()

    loop = GLib.MainLoop()
    wc.request_async('https://www.gnome.org/', None, check_result, loop)
    loop.run()

_init()
