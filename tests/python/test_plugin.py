import unittest
import logging

try:
    from gi.repository import Grl
except:
    logging.warning("Unable to import Grilo's introspection bindings")
    exit()

class TestMediaPlugin(unittest.TestCase):

    NONEXISTING_KEY = 'nonexisting_key'

    def __init__(self, method_name):
        super(TestMediaPlugin, self).__init__(method_name)
        Grl.init([])
        self.registry = Grl.Registry.get_default()
        self.registry.load_all_plugins()
        sources = self.registry.get_sources(False)
        if sources:
            self.plugin = sources[0]

    def test_get_name(self):
        name = self.plugin.get_name()
        self.assertTrue(name)

    def test_get_description(self):
        description = self.plugin.get_name()
        self.assertTrue(description)

    def test_get_version(self):
        version = self.plugin.get_version()
        self.assertTrue(version)

    def test_get_license(self):
        license = self.plugin.get_license()
        self.assertTrue(license)

    def test_get_author(self):
        author = self.plugin.get_author()
        self.assertTrue(author)

    def test_get_site(self):
        site = self.plugin.get_site()
        self.assertTrue(site)

    def test_get_id(self):
        id = self.plugin.get_id()
        self.assertTrue(id)

    def test_get_filename(self):
        filename = self.plugin.get_filename()
        self.assertTrue(filename)

    def test_get_rank(self):
        rank = self.plugin.get_rank()
        self.assertTrue(rank is not None)

    def test_get_info_keys(self):
        info_keys = self.plugin.get_info_keys()
        self.assertTrue(info_keys)

    def test_get_info_existing_key(self):
        keys = self.plugin.get_info_keys()
        if keys:
            existing_key = keys[0]
            info = self.plugin.get_info(existing_key)
            self.assertTrue(info)

    def test_get_info_nonexisting_key(self):
        info = self.plugin.get_info(self.NONEXISTING_KEY)
        self.assertFalse(info)
