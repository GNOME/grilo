import unittest
import logging
import constants

try:
    from gi.repository import Grl
except:
    logging.warning("Unable to import Grilo's introspection bindings")

class TestMetadataSource(unittest.TestCase):

    MEDIA = Grl.Media()
    METADATA_FLAGS = Grl.MetadataResolutionFlags(Grl.MetadataResolutionFlags.FULL |
                                                 Grl.MetadataResolutionFlags.IDLE_RELAY)

    def __init__(self, method_name):
        super(TestMetadataSource, self).__init__(method_name)
        Grl.init([])
        self.registry = Grl.Registry.get_default()
        self.registry.load_all()
        ops = Grl.SupportedOps(Grl.SupportedOps.RESOLVE |
                               Grl.SupportedOps.SET_METADATA)
        sources = self.registry.get_sources_by_operations(ops, False)
        if sources:
            self.metadata_source = sources[0]

    def test_supported_ops(self):
        ops = self.metadata_source.supported_operations()
        self.assertTrue(ops)

    def test_supported_keys(self):
        keys = self.metadata_source.supported_keys()
        self.assertTrue(keys)

    def test_slow_keys(self):
        keys = self.metadata_source.slow_keys()

    def test_filter_supported(self):
        sources = self.registry.get_sources(False)
        keys = self.registry.get_metadata_keys()
        for source in sources:
            supported, unsupported = source.filter_supported(keys, True)
            self.assertEqual(len(supported) + len(unsupported), len(keys))

    def test_filter_slow(self):
        sources = self.registry.get_sources(False)
        keys = self.registry.get_metadata_keys()
        for source in sources:
            fast, slow = source.filter_slow(keys, True)
            self.assertEqual(len(fast) + len(slow), len(keys))

    def test_filter_writable(self):
        sources = self.registry.get_sources(False)
        keys = self.registry.get_metadata_keys()
        for source in sources:
            writable, unwritable = source.filter_writable(keys, True)
            self.assertEqual(len(writable) + len(unwritable), len(keys))

    def test_key_depends(self):
        key_id = self.registry.lookup_metadata_key(constants.KEY_ID)
        keys = self.metadata_source.key_depends(key_id)
        self.assertTrue(keys is not None)

    def test_writable_keys(self):
        keys = self.metadata_source.writable_keys()
        self.assertTrue(keys is not None)

    def test_resolve(self):
        self.metadata_source.resolve([], self.MEDIA, self.METADATA_FLAGS,
                                     self.resolve_cb, None)

    def resolve_cb(self, *args):
        pass

    def test_resolve_sync(self):
        try:
            self.metadata_source.resolve_sync([], self.MEDIA, self.METADATA_FLAGS)
        except Exception, ex:
            self.fail(ex)

    def test_set_metadata(self):
        self.metadata_source.set_metadata(self.MEDIA, [], self.METADATA_FLAGS,
                                          self.metadata_cb, None)

    def metadata_cb(self, *args):
        pass

    def test_set_metadata_sync(self):
        try:
            self.metadata_source.set_metadata_sync(self.MEDIA,
                                                   self.registry.get_metadata_keys(),
                                                   self.METADATA_FLAGS)
        except Exception, ex:
            self.fail(ex)

    def test_get_id(self):
        id = self.metadata_source.get_id()
        self.assertTrue(id)

    def test_get_name(self):
        name = self.metadata_source.get_name()
        self.assertTrue(name)

    def test_get_description(self):
        description = self.metadata_source.get_description()
        self.assertTrue(description)
