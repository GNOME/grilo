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

OPTION_TYPE_FILTER = "type-filter"

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

    def test_type_filter(self):
        caps = Grl.Caps()
        # test default value
        self.assertEqual(caps.get_type_filter(), Grl.TypeFilter.NONE)

        self.assertTrue(caps.test_option(OPTION_TYPE_FILTER, Grl.TypeFilter.NONE))
        self.assertFalse(caps.test_option(OPTION_TYPE_FILTER, Grl.TypeFilter.AUDIO))

        caps.set_type_filter(Grl.TypeFilter.VIDEO)
        self.assertEqual(caps.get_type_filter(), Grl.TypeFilter.VIDEO)
        self.assertTrue(caps.test_option(OPTION_TYPE_FILTER, Grl.TypeFilter.NONE))
        self.assertTrue(caps.test_option(OPTION_TYPE_FILTER, Grl.TypeFilter.VIDEO))
        self.assertFalse(caps.test_option(OPTION_TYPE_FILTER, Grl.TypeFilter.AUDIO))
        self.assertFalse(caps.test_option(OPTION_TYPE_FILTER,
                                          Grl.TypeFilter.AUDIO | Grl.TypeFilter.VIDEO))

    def test_key_filter(self):
        caps = Grl.Caps()
        self.assertEqual(caps.get_key_filter(), [])
        keys = [Grl.METADATA_KEY_ARTIST, Grl.METADATA_KEY_ALBUM]
        caps.set_key_filter(keys)
        self.assertEqual(caps.get_key_filter(), keys)
        self.assertTrue(caps.is_key_filter(Grl.METADATA_KEY_ARTIST))
        self.assertTrue(caps.is_key_filter(Grl.METADATA_KEY_ALBUM))
        self.assertFalse(caps.is_key_filter(Grl.METADATA_KEY_INVALID))
        self.assertFalse(caps.is_key_filter(Grl.METADATA_KEY_MODIFICATION_DATE))


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
        self.assertEqual(options.get_type_filter(), Grl.TypeFilter.ALL)


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

        fltr = Grl.TypeFilter.AUDIO | Grl.TypeFilter.VIDEO
        options.set_type_filter(fltr)
        self.assertEqual(options.get_type_filter(), fltr)

    def test_caps(self):
        caps = Grl.Caps()
        caps.set_type_filter(Grl.TypeFilter.VIDEO)
        options = Grl.OperationOptions.new(caps)
        self.assertTrue(options.set_type_filter(Grl.TypeFilter.VIDEO))
        self.assertFalse(options.set_type_filter(Grl.TypeFilter.AUDIO))
        self.assertFalse(options.set_type_filter(Grl.TypeFilter.IMAGE))

        # now with other caps, test obey_caps()
        caps2 = Grl.Caps()
        caps2.set_type_filter(Grl.TypeFilter.AUDIO)
        ret, supported, unsupported = options.obey_caps(caps2)
        self.assertFalse(ret)
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_SKIP))
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_COUNT))
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_FLAGS))
        self.assertFalse(supported.key_is_set(OPTION_TYPE_FILTER))

        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_SKIP))
        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_COUNT))
        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_FLAGS))
        self.assertTrue(unsupported.key_is_set(OPTION_TYPE_FILTER))
        self.assertEqual(unsupported.get_type_filter(), Grl.TypeFilter.VIDEO)

        caps3 = Grl.Caps()
        caps3.set_type_filter(Grl.TypeFilter.VIDEO)
        ret, supported, unsupported = options.obey_caps(caps3)
        self.assertTrue(ret)
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_SKIP))
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_COUNT))
        self.assertFalse(supported.key_is_set(Grl.OPERATION_OPTION_FLAGS))
        self.assertTrue(supported.key_is_set(OPTION_TYPE_FILTER))
        self.assertEqual(supported.get_type_filter(), Grl.TypeFilter.VIDEO)

        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_SKIP))
        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_COUNT))
        self.assertFalse(unsupported.key_is_set(Grl.OPERATION_OPTION_FLAGS))
        self.assertFalse(unsupported.key_is_set(OPTION_TYPE_FILTER))

    def test_key_filters(self):
        options = Grl.OperationOptions.new(None)
        release_date = GLib.DateTime.new_utc(1994,1,1,12,0,0)
        self.assertEqual(options.get_key_filter_list(), [])
        self.assertTrue(options.set_key_filter_value(Grl.METADATA_KEY_ARTIST, "NOFX"))
        self.assertTrue(options.set_key_filters({
                                                Grl.METADATA_KEY_ALBUM: "Punk in Drublic",
                                                Grl.METADATA_KEY_CREATION_DATE: release_date}))
        filter_list = options.get_key_filter_list()
        filter_list.sort()
        expected_filters = [Grl.METADATA_KEY_ARTIST,
                            Grl.METADATA_KEY_ALBUM,
                            Grl.METADATA_KEY_CREATION_DATE]
        expected_filters.sort()
        self.assertEqual(filter_list, expected_filters)
        self.assertEqual(options.get_key_filter(Grl.METADATA_KEY_ARTIST), "NOFX")
        self.assertEqual(options.get_key_filter(Grl.METADATA_KEY_ALBUM), "Punk in Drublic")
        filter_date = options.get_key_filter(Grl.METADATA_KEY_CREATION_DATE)
        self.assertTrue(filter_date is not None)
        self.assertEqual(filter_date.format("%FT%H:%M:%S:%Z"), release_date.format("%FT%H:%M:%S:%Z"))

    def test_key_filters_with_caps(self):
        caps = Grl.Caps()
        release_date = GLib.DateTime.new_utc(1994,1,1,12,0,0)
        caps.set_key_filter([Grl.METADATA_KEY_ARTIST, Grl.METADATA_KEY_CREATION_DATE])
        options = Grl.OperationOptions.new(caps)
        self.assertTrue(options.set_key_filter_value(Grl.METADATA_KEY_ARTIST, "NOFX"))
        self.assertFalse(options.set_key_filter_value(Grl.METADATA_KEY_ALBUM, "Punk in Drublic"))
        self.assertTrue(options.set_key_filter_value(Grl.METADATA_KEY_CREATION_DATE, release_date))
        filter_list = options.get_key_filter_list()
        filter_list.sort()
        expected_filters = [Grl.METADATA_KEY_ARTIST,
                            Grl.METADATA_KEY_CREATION_DATE]
        expected_filters.sort()
        self.assertEqual(filter_list, expected_filters)
        self.assertEqual(options.get_key_filter(Grl.METADATA_KEY_ARTIST), "NOFX")
        self.assertEqual(options.get_key_filter(Grl.METADATA_KEY_ALBUM), None)

        filter_date = options.get_key_filter(Grl.METADATA_KEY_CREATION_DATE)
        self.assertTrue(filter_date is not None)
        self.assertEqual(filter_date.format("%FT%H:%M:%S:%Z"), release_date.format("%FT%H:%M:%S:%Z"))



class TestFileSystem(unittest.TestCase):
    def __init__(self, method_name):
        super(TestFileSystem, self).__init__(method_name)
        self.registry = Grl.Registry.get_default()
        self.plugin = self.registry.lookup_source("grl-filesystem")

    def test_caps(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.BROWSE)
        self.assertFalse(caps is None)


# who said "this is ugly" ?
Grl.init([])
registry = Grl.Registry.get_default()
try:
    registry.load_plugin_by_id("grl-filesystem")
except GLib.Error:
    pass
