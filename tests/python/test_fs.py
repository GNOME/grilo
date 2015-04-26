# some tests on filesystem, useful to test core stuff as long as we don't have
# proper mock sources.
import unittest
import os, tempfile, shutil, time, calendar

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



_tempdir = tempfile.mkdtemp()


class TestFSOperations(unittest.TestCase):
    def __init__(self, method_name):
        super(TestFSOperations, self).__init__(method_name)
        self.registry = Grl.Registry.get_default()
        self.plugin = self.registry.lookup_source("grl-filesystem")

        self.file_tree_pictures = [
            "a/aa/a.jpg",
            "a/ab/a.jpg",
            "x/yz/a.png",
            "a/a.bmp",
        ]
        self.file_tree_audio = [
            "a/aa/a.mp3",
            "a/ab/a.mp3",
            "a/a.ogg",
        ]
        self.file_tree_video = [
            "a/aa/aa7.avi",
            "x/yz/aa8.mpeg",
            "a/ab/aa9.mkv",
        ]

        self.file_tree = self.file_tree_pictures \
                         + self.file_tree_audio \
                         + self.file_tree_video



    def _touch_file(self, path):
        dirname = os.path.dirname(path)
        if not os.path.isdir(dirname):
            os.makedirs(dirname)
        open(path, 'w')

    def setUp(self):
        file_tree =  (os.path.join(_tempdir, d) for d in self.file_tree)

        for f in file_tree:
            self._touch_file(f)

        self._old_files = [os.path.join(_tempdir, f) \
                          for f in (self.file_tree_pictures[1],
                                    self.file_tree_audio[1],
                                    self.file_tree_video[1])]
        for f in self._old_files:
            mtime = calendar.timegm(time.strptime("1989-07-14", "%Y-%m-%d"))
            os.utime(f, (mtime, mtime))

    def tearDown(self):
        if _tempdir and os.path.isdir(_tempdir):
            shutil.rmtree(_tempdir)

    def test_search(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        l = self.plugin.search_sync("a", [Grl.METADATA_KEY_ID], options)
        self.assertNotEqual(l, None)
        expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree)
        path_list = [m.get_id() for m in l]
        path_list.sort()
        self.assertEqual(path_list, expected_result)

    def test_type_filtered_search(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        options.set_type_filter(Grl.TypeFilter.VIDEO)
        l = self.plugin.search_sync("a", [Grl.METADATA_KEY_ID], options)
        self.assertNotEqual(l, None)
        expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree_video)
        path_list = sorted(m.get_id() for m in l)
        self.assertEqual(path_list, expected_result)

    def test_key_filtered_search(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        options.set_key_filters({Grl.METADATA_KEY_MIME: 'image/jpeg'})
        l = self.plugin.search_sync("a", [Grl.METADATA_KEY_ID, Grl.METADATA_KEY_MIME], options)
        expected_result = sorted(os.path.join(_tempdir, d)
                                 for d in self.file_tree_pictures
                                 if d.endswith('.jpg'))
        path_list = sorted(m.get_id() for m in l)
        self.assertEqual(path_list, expected_result)

    def test_key_range_filtered_search(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        # For now, we provide date _and_ time because of
        # https://bugzilla.gnome.org/show_bug.cgi?id=650968
        start = Grl.date_time_from_iso8601("1989-07-13T00:00:00Z")
        end = Grl.date_time_from_iso8601("1989-07-15T00:00:00Z")
        start.ref()
        end.ref()
        options.set_key_range_filter_value(Grl.METADATA_KEY_MODIFICATION_DATE,
                                           start, end)
        l = self.plugin.search_sync("a",
                                    [Grl.METADATA_KEY_ID,
                                    Grl.METADATA_KEY_MODIFICATION_DATE],
                                    options)
        expected_result = sorted(self._old_files)
        path_list = sorted(m.get_id() for m in l)
        self.assertEqual(path_list, expected_result)


    def test_browse(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        #self._results = []
        results = []
        def recursive_browse(box, results=results):
            new_results = self.plugin.browse_sync(box, [Grl.METADATA_KEY_ID], options)
            results += new_results
            for m in new_results:
                if isinstance(m, Grl.MediaBox):
                    recursive_browse(m)

        recursive_browse(None)
        path_list = sorted(m.get_id() for m in results if not isinstance(m, Grl.MediaBox))
        expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree)
        self.assertEqual(path_list, expected_result)

    def test_search_async(self):
        l = []
        def callback(source, operation_id, media, remaining, user_data, error):
            try:
                if media:
                    l.append(media)
                if remaining == 0:
                    self.assertNotEqual(l, None)
                    expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree)
                    path_list = [m.get_id() for m in l]
                    path_list.sort()
                    self.assertEqual(path_list, expected_result)
            finally:
                if remaining == 0:
                    loop.quit()

        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        self.plugin.search("a", [Grl.METADATA_KEY_ID], options, callback, None)

        loop = GLib.MainLoop()
        loop.run()

    def test_type_filtered_search_async(self):
        l = []
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        options.set_type_filter(Grl.TypeFilter.VIDEO)

        def callback(source, operation_id, media, remaining, user_data, error):
            try:
                if media:
                    l.append(media)
                if remaining == 0:
                    self.assertNotEqual(l, None)
                    expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree_video)
                    path_list = sorted(m.get_id() for m in l)
                    self.assertEqual(path_list, expected_result)
            finally:
                if remaining == 0:
                    loop.quit()


        self.plugin.search("a", [Grl.METADATA_KEY_ID], options, callback, None)

        loop = GLib.MainLoop()
        loop.run()

    def test_key_filtered_search_async(self):
        l = []
        def callback(source, operation_id, media, remaining, user_data, error):
            try:
                if media:
                    l.append(media)
                if remaining == 0:
                    expected_result = sorted(os.path.join(_tempdir, d)
                                             for d in self.file_tree_pictures
                                             if d.endswith('.jpg'))
                    path_list = sorted(m.get_id() for m in l)
                    self.assertEqual(path_list, expected_result)
            finally:
                if remaining == 0:
                    loop.quit()

        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        options.set_key_filters({Grl.METADATA_KEY_MIME: 'image/jpeg'})
        self.plugin.search("a", [Grl.METADATA_KEY_ID, Grl.METADATA_KEY_MIME], options, callback, None)

        loop = GLib.MainLoop()
        loop.run()

    def test_browse_async(self):
        caps = self.plugin.get_caps(Grl.SupportedOps.SEARCH)
        options = Grl.OperationOptions.new(caps)
        l = []
        results = []
        def callback(source, operation_id, media, remaining, user_data, error):
            if media:
                l.append(media)
            if remaining == 0:
                # todo: recurse
                dirs = []
                for m in l:
                    if isinstance(m, Grl.MediaBox):
                        dirs.append(m)
                    else:
                        results.append(m)
                l[:] = []
                for m in dirs:
                    self.plugin.browse(m, [Grl.METADATA_KEY_ID], options, callback, None)


        def check_result():
            loop.quit()
            path_list = sorted(m.get_id() for m in results)
            expected_result = sorted(os.path.join(_tempdir, d) for d in self.file_tree)
            self.assertEqual(path_list, expected_result)

            return False

        self.plugin.browse(None, [Grl.METADATA_KEY_ID], options, callback, None)

        loop = GLib.MainLoop()
        GLib.timeout_add(1000, check_result)
        loop.run()


def _init():
    Grl.init([])
    registry = Grl.Registry.get_default()

    fs_config = Grl.Config.new("grl-filesystem", None)
    fs_config.set_string("base-path", _tempdir)
    registry.add_config(fs_config)

    try:
        registry.load_plugin_by_id("grl-filesystem")
    except GLib.Error:
        pass

_init()
