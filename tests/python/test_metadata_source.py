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

    def setUp(self):
        Grl.init([])
        self.registry = Grl.PluginRegistry.get_default()
        self.registry.load_all()
        ops = Grl.SupportedOps(Grl.SupportedOps.RESOLVE |
                               Grl.SupportedOps.SET_METADATA)
        sources = self.registry.get_sources_by_operations(ops, False)
        if sources:
            self.metadata_source = sources[0]

    def tearDown(self):
        self.metadata_source = None
        for source in self.registry.get_sources(False):
            self.registry.unload(source.get_id())
            self.registry.unregister_source(source)

    def test_supported_ops(self):
        ops = self.metadata_source.supported_operations()
        self.assertTrue(ops)

    def test_supported_keys(self):
        keys = self.metadata_source.supported_keys()
        self.assertTrue(keys)

    def test_slow_keys(self):
        keys = self.metadata_source.slow_keys()

    def test_filter_supported(self):
        keys = self.metadata_source.filter_supported([], True)

    def test_filter_slow(self):
        keys = self.metadata_source.filter_slow([], True)

    def test_filter_writable(self):
        keys = self.metadata_source.filter_writable([], True)

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
