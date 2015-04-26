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

class TestMedia(unittest.TestCase):
    def test_creation(self):
        media = Grl.Media.new()
        self.assertFalse(media is None)

    def test_publication_date(self):
        media = Grl.Media.new()
        date_string = "2001-02-03T04:05:06Z"
        date = Grl.date_time_from_iso8601 (date_string)

        self.assertTrue(media.get_publication_date() is None)
        media.set_publication_date(date)
        d = media.get_publication_date()
        self.assertTrue(d is not None)
        self.assertEqual(d.get_year(), 2001)
        self.assertEqual(d.get_month(), 2)
        self.assertEqual(d.get_day_of_month(), 3)
        self.assertEqual(d.get_hour(), 4)
        self.assertEqual(d.get_minute(), 5)
        self.assertEqual(d.get_second(), 6)
        self.assertEqual(d.format("%FT%H:%M:%S%Z"), date.format("%FT%H:%M:%S%Z"))

        # date only
        date_string = "2001-02-03"
        date = Grl.date_time_from_iso8601 (date_string)
        media.set_publication_date(date)
        d = media.get_publication_date()
        self.assertTrue(d is not None)
        self.assertEqual(d.get_year(), 2001)
        self.assertEqual(d.get_month(), 2)
        self.assertEqual(d.get_day_of_month(), 3)
        self.assertEqual(d.get_hour(), 12)
        self.assertEqual(d.get_minute(), 0)
        self.assertEqual(d.get_second(), 0)
        self.assertEqual(d.format("%FT%H:%M:%S%Z"), date.format("%FT%H:%M:%S%Z"))
        self.assertEqual(d.format("%FT%H:%M:%S%Z"), date_string + "T12:00:00UTC")

# who said "this is ugly" ?
Grl.init([])
registry = Grl.Registry.get_default()
try:
    registry.load_plugin_by_id("grl-filesystem")
except GLib.Error:
    pass
