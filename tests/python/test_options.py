import unittest
import logging

try:
    from gi.repository import Grl
except:
    logging.warning("Unable to import Grilo's introspection bindings")
    exit()
try:
    from gi.repository import GLib
except TypeError:
    # there seems to be a bug with GVariants these days that triggers an
    # exception when importing GLib. Seems to be harmless though. We import
    # explicitly GLib in a try/except here so that we can handle that, else the
    # exception would be raised the first time it is imported by other code
    # (e.g. by using a GDateTime)
    from gi.repository import GLib


class TestCaps(unittest.TestCase):
    def test_creation(self):
        caps = Grl.Caps()
        self.assertFalse(caps is None)

    def test_mandatory_options(self):
        return None
        caps = Grl.Caps()
        for key in ("skip", "count", "flags"):
            self.assertTrue(caps.test_option(key, 0),
                            "test_option() returned False for %s" % key)


class TestOptions(unittest.TestCase):
    def test_creation(self):
        caps = Grl.Caps()
        self.assertFalse(caps is None)
        options = Grl.OperationOptions.new(caps)
        self.assertFalse(options is None)
        self.assertFalse(Grl.OperationOptions() is None)

    def test_default_values(self):
        options = Grl.OperationOptions()
        self.assertEqual(options.get_skip(), 0)
        self.assertEqual(options.get_count(), Grl.COUNT_INFINITY)
        self.assertEqual(options.get_flags(),
                         Grl.MetadataResolutionFlags.NORMAL)


    def test_value_setting_no_caps(self):
        options = Grl.OperationOptions()

        options.set_skip(12)
        self.assertEqual(options.get_skip(), 12)

        options.set_count(28)
        self.assertEqual(options.get_count(), 28)

        flags =Grl.MetadataResolutionFlags.FAST_ONLY \
                | Grl.MetadataResolutionFlags.IDLE_RELAY
        options.set_flags(flags)
        self.assertEqual(options.get_flags(), flags)


class TestFileSystem(unittest.TestCase):
    def __init__(self, method_name):
        super(TestFileSystem, self).__init__(method_name)
        self.registry = Grl.PluginRegistry.get_default()
        self.plugin = self.registry.lookup_source("grl-filesystem")

    def test_caps(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.BROWSE)
        self.assertFalse(caps is None)


# who said "this is ugly" ?
Grl.init([])
registry = Grl.PluginRegistry.get_default()
registry.load_by_id("grl-filesystem")
