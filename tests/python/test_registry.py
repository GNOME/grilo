import unittest
import os
import util
import tempfile
import logging
import constants
import glib

try:
    from gi.repository import Grl
except:
    logging.warning("Unable to import Grilo's introspection bindings")
    exit()

class TestRegistry(unittest.TestCase):

    EXISTING_LIBRARY_PATH = None

    NONEXISTING_LIBRARY_PATH = 'NONEXISTING_LIBRARY.so'

    INVALID_LIBRARY_PATH = os.path.join(util.PREFIX,
                                        util.GRL_LIB_NAME)

    NONEXISTING_SOURCE = 'NON_EXISTING_SOURCE'

    def __init__(self, method_name):
        super(TestRegistry, self).__init__(method_name)
        Grl.init([])
        self.registry = Grl.Registry.get_default()
        plugin_paths = util.GRL_PLUGIN_PATH.split(':')
        for path in plugin_paths:
            if path:
                entries = os.listdir(path)
                self.EXISTING_LIBRARY_PATH = os.path.join(path,
                                                          entries[0])
                break
        print self.EXISTING_LIBRARY_PATH

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_get_default_not_null(self):
        registry = Grl.Registry.get_default()
        self.assertTrue(registry)

    def test_get_default_singleton(self):
        registry1 = Grl.Registry.get_default()
        registry2 = Grl.Registry.get_default()
        self.assertEquals(registry1, registry2)

    def test_add_directory(self):
        pass

    def test_load_existing(self):
        # skipping since this is likely to have been loaded by another test
        return None

        if not self.registry.load(self.EXISTING_LIBRARY_PATH):
            self.fail()

    def test_load_unexisting(self):
        self.assertRaises(glib.GError, self.registry.load, self.NONEXISTING_LIBRARY_PATH)

    def test_load_invalid(self):
        self.assertRaises(glib.GError, self.registry.load, self.INVALID_LIBRARY_PATH)

    def test_load_directory_nonexisting(self):
        self.assertRaises(glib.GError, self.registry.load_directory, '')

    def test_load_directory_existing(self):
        if not self.registry.load_directory(os.getcwd()):
            self.fail()

    def test_unload(self):
        pass

    def test_load_all(self):
        self.assertTrue(self.registry.load_all())

    def test_register_source(self):
        pass

    def test_unregister_source(self):
        self.registry.load_all()
        sources = self.registry.get_sources(False)
        for source in sources:
            self.registry.unregister_source(source)
        sources = self.registry.get_sources(False)
        self.assertEqual(len(sources), 0)

    def test_lookup_source(self):
        self.registry.load_all()
        sources = self.registry.get_sources(False)

        if sources:
            expected = sources[0]
            search_id = expected.get_id()
            found = self.registry.lookup_source(search_id)

            self.assertEqual(expected, found)

    def test_lookup_source_nonexisting(self):
        self.registry.load_all()
        found = self.registry.lookup_source(self.NONEXISTING_SOURCE)
        self.assertFalse(found)

    def test_get_sources(self):
        sources = self.registry.get_sources(False)
        self.assertEqual(len(sources), 0)

        self.registry.load_all()
        sources = self.registry.get_sources(False)
        self.assertNotEqual(len(sources), 0)

    def test_get_sources_ordering(self):
        self.registry.load_all()
        ordered_sources = self.registry.get_sources(True)
        unordered_sources = self.registry.get_sources(False)

        unordered_sources.sort(key=Grl.MediaPlugin.get_rank)
        self.assertEquals(ordered_sources, unordered_sources)

    def test_get_sources_by_operations_supported_ops(self):
        self.registry.load_all()
        for op in constants.SUPPORTED_OPS:
            if op != Grl.SupportedOps.NONE:
                sources = self.registry.get_sources_by_operations(op, False)
                if sources:
                    supported_operations = sources[0].supported_operations()
                    self.assertTrue(supported_operations & op)

    def test_register_metadata_key(self):
        pass

    def test_lookup_metadata_key_existing_key(self):
        existing_key = self.registry.lookup_metadata_key(constants.KEY_ID)
        self.assertTrue(existing_key)

    def test_lookup_metadata_key_nonexisting_key(self):
        nonexisting_key = self.registry.lookup_metadata_key(constants.KEY_NONEXISTING)
        self.assertTrue(nonexisting_key == Grl.METADATA_KEY_INVALID)

    def test_lookup_metadata_key_singleton(self):
        a_key = self.registry.lookup_metadata_key(constants.KEY_ID)
        another_key = self.registry.lookup_metadata_key(constants.KEY_ID)
        self.assertEquals(a_key, another_key)

    def test_get_metadata_keys_contains(self):
        registered_key = self.registry.lookup_metadata_key(constants.KEY_ARTIST)
        metadata_keys = self.registry.get_metadata_keys()
        self.assertTrue(registered_key in metadata_keys)

    def test_get_metadata_keys_all_keys(self):
        registered_keys = []
        for k in constants.REGISTERED_KEYS:
            registered_keys.append(self.registry.lookup_metadata_key(k))

        metadata_keys = self.registry.get_metadata_keys()
        self.assertEquals(registered_keys.sort(), metadata_keys.sort())

    def test_add_config(self):
        pass
