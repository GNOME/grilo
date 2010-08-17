#!/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2010 Igalia S.L.
#
# Contact: Simón Pena <spenap@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA
#

import pygtk
pygtk.require('2.0')

from gi.repository import Grl
from gi.repository import Gtk
from gi.repository import Gio
from gi.repository import GdkPixbuf

class MainWindow(Gtk.Window):

    BROWSE_FLAGS = (Grl.MetadataResolutionFlags) (Grl.MetadataResolutionFlags.FAST_ONLY |
                                                  Grl.MetadataResolutionFlags.IDLE_RELAY)
    METADATA_FLAGS = (Grl.MetadataResolutionFlags) (Grl.MetadataResolutionFlags.FULL |
                                                    Grl.MetadataResolutionFlags.IDLE_RELAY)
    BROWSE_CHUNK_SIZE = 100
    BROWSE_MAX_COUNT = 2 * BROWSE_CHUNK_SIZE

    BROWSE_KEYS = []
    METADATA_KEYS = []

    def __init__(self):
        super(MainWindow, self).__init__(type=Gtk.WindowType.TOPLEVEL)
        self.connect('destroy', self._quit)
        self.set_title('Grilo Test UI using introspection')

        Grl.init([])

        self._ui_state = UIState()
        self.BROWSE_KEYS = self._lookup_browse_keys()
        self.METADATA_KEYS = self._lookup_metadata_keys()
        self._launchers = UriLaunchers()

        self._setup_ui()
        self._load_plugins()

        self.show_all()

    def _lookup_browse_keys(self):
        registry = Grl.PluginRegistry.get_default()
        key_id = registry.lookup_metadata_key('id')
        key_title = registry.lookup_metadata_key('title')
        key_childcount = registry.lookup_metadata_key('childcount')
        return [key_id, key_title, key_childcount]

    def _lookup_metadata_keys(self):
        registry = Grl.PluginRegistry.get_default()
        return registry.get_metadata_keys()

    def _load_plugins(self):
        registry = Grl.PluginRegistry.get_default()
        registry.connect('source-added',
                         self._source_added_cb)
        registry.connect('source-removed',
                         self._source_removed_cb)
        registry.load_all()

    def _setup_ui(self):
        main_box = Gtk.HPaned()
        left_pane = Gtk.VBox()
        right_pane = Gtk.VBox()

        main_box.add(left_pane)
        main_box.add(right_pane)
        self.add(main_box)

        hbox = Gtk.HBox()
        vbox = Gtk.VBox()
        self._search_text = Gtk.Entry()
        vbox.add(self._search_text)
        self._query_text = Gtk.Entry()
        vbox.add(self._query_text)
        hbox.add(vbox)

        vbox = Gtk.VBox()
        self._search_combo = SearchComboBox()
        vbox.pack_start(self._search_combo, False, False, 0)

        self._query_combo = QueryComboBox()
        vbox.pack_start(self._query_combo, False, False, 0)
        hbox.add(vbox)

        vbox = Gtk.VBox()
        search_btn = self._create_button('Search', self._search_btn_clicked_cb)
        vbox.pack_start(search_btn, False, False, 0)
        query_btn = self._create_button('Query', self._query_btn_clicked_cb)
        vbox.pack_start(query_btn, False, False, 0)
        hbox.add(vbox)

        left_pane.pack_start(hbox, False, False, 0)

        toolbar_buttons = self._create_toolbar_buttons()
        left_pane.pack_start(toolbar_buttons, False, False, 0)

        self._browser_window = self._create_browser_scrolled_window()
        left_pane.add(self._browser_window)

        self._show_btn = self._create_button('Show', self._show_btn_clicked_cb)
        self._show_btn.set_sensitive(False)
        right_pane.pack_start(self._show_btn, False, False, 0)

        self._contents_window = self._create_contents_window()
        right_pane.add(self._contents_window)

        self._show_plugins()

    def _create_button(self, name, callback):
        button = Gtk.Button()
        button.set_label(name)
        button.connect('clicked', callback)
        return button

    def _show_plugins(self):
        registry = Grl.PluginRegistry.get_default()
        self._clear_panes()
        sources = registry.get_sources_by_operations(Grl.SupportedOps.BROWSE, False)
        self._browser_window.get_browser().add_sources(sources)

    def _clear_panes(self):
        browser_model = BrowserListStore()
        self._browser_window.get_browser().set_model(browser_model)

        metadata_model = MetadataListStore()
        self._contents_window.get_metadata().set_model(metadata_model)

        self._ui_state.last_url = None
        self._store_btn.set_sensitive(False)
        self._remove_btn.set_sensitive(False)
        self._show_btn.set_sensitive(False)

    def _create_contents_window(self):
        scrolled_window = ContentScrolledWindow()
        return scrolled_window

    def _create_browser_scrolled_window(self):
        scrolled_window = BrowserScrolledWindow()
        scrolled_window.get_browser().connect('row-activated',
                                              self._browser_activated_cb)
        scrolled_window.get_browser().connect('cursor-changed',
                                              self._browser_row_selected_cb)
        return scrolled_window

    def _create_toolbar_buttons(self):
        toolbar_buttons = Gtk.HBox()
        self._back_btn = Gtk.Button()
        self._back_btn.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_GO_BACK,
                                                    Gtk.IconSize.BUTTON))
        self._store_btn = Gtk.Button()
        self._store_btn.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_ADD,
                                                     Gtk.IconSize.BUTTON))
        self._remove_btn = Gtk.Button()
        self._remove_btn.set_image(Gtk.Image.new_from_stock(Gtk.STOCK_REMOVE,
                                                      Gtk.IconSize.BUTTON))

        toolbar_buttons.pack_start(self._back_btn, False, False, 0)
        toolbar_buttons.pack_start(self._store_btn, False, False, 0)
        toolbar_buttons.pack_start(self._remove_btn, False, False, 0)

        self._back_btn.connect('clicked',
            self._back_btn_clicked_cb)

        self._store_btn.connect('clicked',
            self._store_btn_clicked_cb)

        self._remove_btn.connect('clicked',
            self._remove_btn_clicked_cb)

        self._store_btn.set_sensitive(False)
        self._remove_btn.set_sensitive(False)
        return toolbar_buttons

    def _source_added_cb(self, plugin_registry, media_source):
        self._show_plugins()
        for combo in [self._search_combo, self._query_combo]:
            combo.update()

    def _source_removed_cb(self, plugin_registry, media_source):
        print media_source.get_name()

    def _browser_activated_cb(self, tree_view, path, column, data=None):
        model = tree_view.get_model()
        success, iter = model.get_iter(path)
        if success:
            source = model.get_value(iter, BrowserListStore.SOURCE_COLUMN)
            content = model.get_value(iter, BrowserListStore.CONTENT_COLUMN)
            type = model.get_value(iter, BrowserListStore.TYPE_COLUMN)
        else:
            return

        if type == BrowserListStore.OBJECT_TYPE_MEDIA:
            return
        elif type == BrowserListStore.OBJECT_TYPE_SOURCE:
            container = None
        elif content:
            container = content
        else:
            container = None

        self._browse_history_push(self._ui_state.cur_source,
                                  self._ui_state.cur_container)
        self.browse(source, container)

    def _browser_row_selected_cb(self, tree_view, data=None):
        path, column = tree_view.get_cursor()
        model = self._browser_window.get_browser().get_model()
        success, iter = model.get_iter(path)
        if not success:
            return
        source = model.get_value(iter, BrowserListStore.SOURCE_COLUMN)
        content = model.get_value(iter, BrowserListStore.CONTENT_COLUMN)

        if source != self._ui_state.cur_md_source or \
           content != self._ui_state.cur_md_media:
            self._set_cur_metadata(source, content)
            self.metadata(source, content)

        if not content and \
           source.supported_operations() & Grl.SupportedOps.STORE:
            self._store_btn.set_sensitive(True)
        elif content and isinstance(content, Grl.MediaBox) and \
             source.supported_operations() & Grl.SupportedOps.STORE_PARENT:
            self._store_btn.set_sensitive(True)
        else:
            self._store_btn.set_sensitive(False)

        if content and \
           source.supported_operations() & Grl.SupportedOps.REMOVE:
            self._remove_btn.set_sensitive(True)
        else:
            self._remove_btn.set_sensitive(False)

    def _show_btn_clicked_cb(self, *args):
        if self._ui_state.last_url:
            uri_list = []
            print ('playing %(url)s' % {'url':self._ui_state.last_url})
            uri_list.append(self._ui_state.last_url)

            registry = Grl.PluginRegistry.get_default()
            key_source = registry.lookup_metadata_key('source')
            if isinstance(self._ui_state.cur_md_media, Grl.MediaImage):
                app = self._launchers.eog
            elif self._ui_state.cur_md_media.get(key_source) == 'grl-apple-trailers':
                app = self._launchers.mplayer
            else:
                app = self._launchers.totem

            try:
                app.launch_uris(uri_list, None)
            except:
                print ('Cannot use %(app)s to show %(source)s; using default application' %
                       {'app':app.get_name(),
                        'source':self._ui_state.last_url})
                try:
                    app.launch_default_for_uri(self._ui_state.last_url, None)
                except:
                    print ('Cannot use default application to show %(source)s. Stopping playback' %
                           {'source':self._ui_state.last_url})

    def _back_btn_clicked_cb(self, *args):
        self._cancel_current_operation()

        prev_source, prev_container = self._browse_history_pop()
        self.browse(prev_source, prev_container)

    def _store_btn_clicked_cb(self, *args):
        selection = self._browser_window.get_browser().get_selection()
        success, model, iter = selection.get_selected()
        if success:
            source = model.get_value(iter, BrowserListStore.SOURCE_COLUMN)
            container = model.get_value(iter, BrowserListStore.CONTENT_COLUMN)
#
        dialog = StoreContentDialog(self)
        if dialog.run() == Gtk.ResponseType.OK:
            url = dialog.get_url()
            if url:
                media = Grl.Media()
                media.set_url(url)
            else:
                media = Grl.MediaBox()
            media.set_title(dialog.get_title())
            media.set_description(dialog.get_description())
            source.store(container,
                         media,
                         self._store_cb,
                         None)
        dialog.destroy()

    def _remove_btn_clicked_cb(self, *args):
        selection = self._browser_window.get_browser().get_selection()
        success, model, iter = selection.get_selected()
        if success:
            source = model.get_value(iter, BrowserListStore.SOURCE_COLUMN)
            media = model.get_value(iter, BrowserListStore.CONTENT_COLUMN)

            source.remove(media, self._remove_cb, None)

    def _query_btn_clicked_cb(self, *args):
        success, iter = self._query_combo.get_active_iter()
        if success:
            model = self._query_combo.get_model()
            source = model.get_value(iter, ComboBoxStore.SOURCE_COLUMN)
            text = self._query_text.get_text()
            self.query(source, text)

    def _search_btn_clicked_cb(self, *args):
        (success, iter) = self._search_combo.get_active_iter()
        if success:
            source = self._search_combo.get_model().get_value(iter,
                                                              ComboBoxStore.SOURCE_COLUMN)
            search_text = self._search_text.get_text()
            self.search(source, search_text)

    def _search_cb(self, source, operation_id, media, remaining, data, error):
        state = data
        state.count += 1

        if media:
            icon = None #get_icon_for_media (media);
            name = media.get_title()
            if isinstance(media, Grl.MediaBox):
                childcount = media.get_childcount()
                type = BrowserListStore.OBJECT_TYPE_CONTAINER
                if childcount == Grl.METADATA_KEY_CHILDCOUNT_UNKNOWN:
                    childcount = '?'
                name = ('%(name)s (%(count)s)' % {'name':name,
                                                  'count':childcount})
            else:
                type = BrowserListStore.OBJECT_TYPE_MEDIA
            model = self._browser_window.get_browser().get_model()
            model.append((source, media, type, name, icon))

        if remaining == 0:
            state.offset += state.count
            if not self._ui_state.multiple and \
               state.count >= self.BROWSE_CHUNK_SIZE and \
               state.offset < self.BROWSE_MAX_COUNT:
                state.count = 0
                next_search_id = source.search(state.text,
                                               self.BROWSE_KEYS,
                                               state.offset,
                                               self.BROWSE_CHUNK_SIZE,
                                               self.BROWSE_FLAGS,
                                               self._search_cb,
                                               state)
                self._operation_started(source, next_search_id, False)
            else:
                self._operation_finished()
                print ('**** search finished %(id)s ****' % {'id':operation_id})

    def _browse_cb(self, source, operation_id, media, remaining, data, error):
        state = data
        state.count += 1

        if media:
            icon = None #get_icon_for_media(media)
            name = media.get_title()
            if isinstance(media, Grl.MediaBox):
                childcount = media.get_childcount()
                type = BrowserListStore.OBJECT_TYPE_CONTAINER
                if childcount == Grl.METADATA_KEY_CHILDCOUNT_UNKNOWN:
                    childcount = '?'
                name = ('%(name)s (%(count)s)' % {'name':name,
                                                  'count':childcount})
            else:
                type = BrowserListStore.OBJECT_TYPE_MEDIA

            model = self._browser_window.get_browser().get_model()
            model.append((source, media, type, name, icon))
        if remaining == 0:
            if self._ui_state.cur_op_id == operation_id and \
               media:
                state.offset += state.count
                if state.count >= self.BROWSE_CHUNK_SIZE and \
                   state.offset < self.BROWSE_MAX_COUNT:
                    state.count = 0
                    next_browse_id = source.browse(self._ui_state.cur_container,
                                                   self.BROWSE_KEYS,
                                                   state.offset,
                                                   self.BROWSE_CHUNK_SIZE,
                                                   self.BROWSE_FLAGS,
                                                   self._browse_cb,
                                                   state)
                    self._operation_started(source, next_browse_id, False)
            else:
                self._operation_finished()
                print ('**** browse finished (%(id)s) ****' % {'id':operation_id})

    def _metadata_cb(self, source, media, data, error):
        if media != self._ui_state.cur_md_media:
            return

        metadata_model = MetadataListStore()
        self._contents_window.get_metadata().set_model(metadata_model)

        if media:
            keys = media.get_keys()
            for key in keys:
                value = media.get(key)
                desc = Grl.metadata_key_get_desc(key)
                metadata_model.append((desc, value))
                print ('%(keyname)s: %(keyvalue)s' % {'keyname':desc,
                                                      'keyvalue':value})

            if (isinstance(media, Grl.MediaAudio) or \
                isinstance(media, Grl.MediaVideo) or \
                isinstance(media, Grl.MediaImage)):
                self._ui_state.last_url = media.get_url()
                if self._ui_state.last_url:
                    self._show_btn.set_sensitive(True)
                else:
                    self._show_btn.set_sensitive(False)

    def _store_cb(source, box, media, user_data, error):
        print 'Media stored'

    def _remove_cb(self, source, media, data, error):
        print 'Media removed'
        self._remove_item_from_view (source, media)

    def _remove_item_from_view(self, source, media):
        model = self._browser_window.get_browser().get_model()
        success, iter = model.get_iter_first()
        found = False
        while success and not found:
            iter_source = model.get_value(iter, BrowserListStore.SOURCE_COLUMN)
            iter_media = model.get_value(iter, BrowserListStore.CONTENT_COLUMN)
            if iter_source == source and iter_media == media:
                model.remove(iter)
                found = True
            else:
                success, iter = model.get_iter_next(iter)

    def _browse_history_push (self, source, media):
        self._ui_state.source_stack.append(source)
        self._ui_state.container_stack.append(media)

    def _browse_history_pop(self):
        prev_source, prev_container = (None, None)
        if self._ui_state.source_stack and \
           self._ui_state.container_stack:
            prev_source = self._ui_state.source_stack.pop()
            prev_container = self._ui_state.container_stack.pop()

        return prev_source, prev_container

    def _cancel_current_operation(self):
        if self._ui_state.op_ongoing:
            if not self._ui_state.multiple:
                self._ui_state.cur_op_source.cancel(self._ui_state.cur_op_id)
            else:
                Grl.multiple_cancel(self._ui_state.cur_op_id)
            self._ui_state.op_ongoing = False

    def _operation_started(self, source, search_id, multiple):
        self._ui_state.op_ongoing = True
        self._ui_state.cur_op_source = source
        self._ui_state.cur_op_id = search_id
        self._ui_state.multiple = multiple

    def _operation_finished(self):
        self._ui_state.op_ongoing = False

    def _set_cur_browse(self, source, media):
        self._ui_state.cur_source = source
        self._ui_state.cur_container = media

    def _set_cur_metadata(self, source, media):
        self._ui_state.cur_md_source = source
        self._ui_state.cur_md_media = media

    def metadata(self, source, media):
        if source:
            if source.supported_operations() & Grl.SupportedOps.METADATA:
                source.metadata(media,
                                self.METADATA_KEYS,
                                self.METADATA_FLAGS,
                                self._metadata_cb,
                                None)
            else:
                self._metadata_cb(source, media, None, None)

    def browse(self, source, container):
        if source:
            self._cancel_current_operation()
            self._clear_panes()
            state = OperationState()
            browse_id = source.browse(container,
                                      self.BROWSE_KEYS,
                                      0,
                                      self.BROWSE_CHUNK_SIZE,
                                      self.BROWSE_FLAGS,
                                      self._browse_cb,
                                      state)
            self._operation_started(source, browse_id, False)
        else:
            self._show_plugins()
        self._set_cur_browse(source, container)
        self._set_cur_metadata(None, None)

    def search(self, source, text):
        self._cancel_current_operation()
        state = OperationState(text)
        if source:
            search_id = source.search(text,
                                      self.BROWSE_KEYS,
                                      0,
                                      self.BROWSE_CHUNK_SIZE,
                                      self.BROWSE_FLAGS,
                                      self._search_cb,
                                      state)
        else:
            search_id = Grl.multiple_search(None,
                                            text,
                                            self.BROWSE_KEYS,
                                            self.BROWSE_MAX_COUNT,
                                            self.BROWSE_FLAGS,
                                            self._search_cb,
                                            state)
        self._clear_panes()
        self._operation_started(source, search_id, source is None)

    def query(self, source, text):
        self._cancel_current_operation()
        state = OperationState(text)
        query_id = source.query(text,
                                self.BROWSE_KEYS,
                                0, self.BROWSE_CHUNK_SIZE,
                                self.BROWSE_FLAGS,
                                self._search_cb,
                                state)
        self._clear_panes()
        self._operation_started(source, query_id, False)

    def run(self):
        Gtk.main()

    def _quit(self, *args):
        Gtk.main_quit()

class StoreContentDialog(Gtk.Dialog):
    def __init__(self, parent):
        super(StoreContentDialog, self).__init__(title='Store content',
                                                 parent=parent,
                                                 flags=Gtk.DialogFlags.MODAL |
                                                       Gtk.DialogFlags.DESTROY_WITH_PARENT,
                                                 buttons=(Gtk.STOCK_OK, Gtk.ResponseType.OK,
                                                          Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL))
        self._setup_ui()
        self.show_all()

    def _setup_ui(self):
        self.vbox = self.get_child()

        self._title_entry = Gtk.Entry()
        self._url_entry = Gtk.Entry()
        self._description_entry = Gtk.Entry()

        entries = [self._title_entry, self._url_entry, self._description_entry]
        labels = ['Title:', 'URL:', 'Description:']

        for i in range(3):
            box = Gtk.HBox()
            label = Gtk.Label(label=labels[i])
            entry = entries[i]
            box.add(label)
            box.add(entry)
            self.vbox.add(box)

    def get_title(self):
        return self._title_entry.get_text()

    def get_url(self):
        return self._url_entry.get_text()

    def get_description(self):
        return self._description_entry.get_text()

class UriLaunchers(object):
    def __init__(self):
        self.eog = Gio.app_info_create_from_commandline('eog',
                                                        'Eye of GNOME (eog)',
                                                        Gio.AppInfoCreateFlags.SUPPORTS_URIS)
        self.totem = Gio.app_info_create_from_commandline('totem',
                                                          'Totem',
                                                          Gio.AppInfoCreateFlags.SUPPORTS_URIS)
        self.mplayer = Gio.app_info_create_from_commandline('mplayer -user-agent \"QuickTime\" -cache 5000',
                                                            'The Movie Player (mplayer)',
                                                            (Gio.AppInfoCreateFlags.SUPPORTS_URIS |
                                                             Gio.AppInfoCreateFlags.NEEDS_TERMINAL))

class UIState(object):

    def __init__(self, source_stack=[], container_stack=[], cur_source=None,
                 cur_container=None, cur_md_source=None, cur_md_media=None,
                 op_ongoing=False, cur_op_source=None, cur_op_id= -1,
                 multiple=False, last_url=None):
        self.source_stack = source_stack
        self.container_stack = container_stack
        self.cur_source = cur_source
        self.cur_container = cur_container
        self.cur_md_source = cur_md_source
        self.cur_md_media = cur_md_media
        self.op_ongoing = op_ongoing
        self.cur_op_source = cur_op_source
        self.cur_op_id = cur_op_id
        self.multiple = multiple
        self.last_url = last_url

class OperationState(object):
    def __init__(self, text=None, offset=0, count=0):
        self.text = text
        self.offset = offset
        self.count = count

class TextComboBox(Gtk.ComboBox):
    def __init__(self):
        super(TextComboBox, self).__init__()
        cell = Gtk.CellRendererText()
        self.pack_start(cell, True)
        #After reading pygi demos, found that set_properties doesn't work
        self.add_attribute(cell, 'text', ComboBoxStore.NAME_COLUMN)

    def update(self, operation):
        registry = Grl.PluginRegistry.get_default()
        sources = registry.get_sources_by_operations(operation,
                                                     False)
        model = ComboBoxStore()
        self.set_model(model)
        model.add(sources)
        self.set_active(0)

class SearchComboBox(TextComboBox):
    def __init__(self):
        super(SearchComboBox, self).__init__()

    def update(self):
        super(SearchComboBox, self).update(Grl.SupportedOps.SEARCH)

class QueryComboBox(TextComboBox):
    def __init__(self):
        super(QueryComboBox, self).__init__()

    def update(self):
        super(QueryComboBox, self).update(Grl.SupportedOps.QUERY)

class ComboBoxStore(Gtk.ListStore):

    NAME_COLUMN = 0
    SOURCE_COLUMN = 1

    def __init__(self):
        super(ComboBoxStore, self).__init__(str,
                                            'GObject')

    def add(self, sources):
        self.clear()
        for source in sources:
            row = {self.NAME_COLUMN: source.get_name(),
                   self.SOURCE_COLUMN: source,
                  }
            self.append(row.values())

class BrowserScrolledWindow(Gtk.ScrolledWindow):
    def __init__(self):
        super(BrowserScrolledWindow, self).__init__()
        self.set_policy(Gtk.PolicyType.AUTOMATIC,
                        Gtk.PolicyType.AUTOMATIC)
        self._browser = BrowserTreeView()
        self.add(self._browser)

    def get_browser(self):
        return self._browser

class ContentScrolledWindow(Gtk.ScrolledWindow):
    def __init__(self):
        super(ContentScrolledWindow, self).__init__()
        self.set_policy(Gtk.PolicyType.AUTOMATIC,
                        Gtk.PolicyType.AUTOMATIC)
        self._metadata = MetadataTreeView()
        self.add(self._metadata)

    def get_metadata(self):
        return self._metadata

class BrowserListStore(Gtk.ListStore):

    SOURCE_COLUMN = 0
    CONTENT_COLUMN = 1
    TYPE_COLUMN = 2
    NAME_COLUMN = 3
    ICON_COLUMN = 4

    OBJECT_TYPE_SOURCE = 0
    OBJECT_TYPE_CONTAINER = 1
    OBJECT_TYPE_MEDIA = 2

    def __init__(self):
        super(BrowserListStore, self).__init__('GObject',
                                               'GObject',
                                               int,
                                               str,
                                               GdkPixbuf.Pixbuf)

    def add(self, sources):
        self.clear()
        for source in sources:
            row = {self.SOURCE_COLUMN: source,
                   self.CONTENT_COLUMN: None,
                   self.TYPE_COLUMN: self.OBJECT_TYPE_SOURCE,
                   self.NAME_COLUMN: source.get_name(),
                   self.ICON_COLUMN: None,
                  }
            self.append(row.values())

class BrowserTreeView(Gtk.TreeView):

    BROWSER_MIN_WIDTH = 320
    BROWSER_MIN_HEIGHT = 400

    def __init__(self):
        super(BrowserTreeView, self).__init__()
        self.set_headers_visible(False)

        model = BrowserListStore()
        self.set_model(model)

        col_attributes = [ 'pixbuf', 'text' ]
        col_model = [ model.ICON_COLUMN, model.NAME_COLUMN ]
        col_renders = [ Gtk.CellRendererPixbuf(),
                        Gtk.CellRendererText()]

        col = Gtk.TreeViewColumn()
        for i in range(2):
            col.pack_start(col_renders[i], False)
            col.add_attribute(col_renders[i], col_attributes[i], col_model[i])

        col.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.insert_column(col, -1)
        self.set_size_request(self.BROWSER_MIN_WIDTH, self.BROWSER_MIN_HEIGHT)

    def add_sources(self, sources=[]):
        model = self.get_model()
        model.add(sources)

class MetadataListStore(Gtk.ListStore):

    NAME_COLUMN = 0
    VALUE_COLUMN = 1

    def __init__(self):
        super(MetadataListStore, self).__init__(str, str)

class MetadataTreeView(Gtk.TreeView):

    METADATA_MIN_WIDTH = 320
    METADATA_MIN_HEIGHT = 400

    def __init__(self):
        super(MetadataTreeView, self).__init__()
        self.set_headers_visible(False)

        model = MetadataListStore()
        self.set_model(model)

        col_renders = [Gtk.CellRendererText(),
                          Gtk.CellRendererText()]
        col_attributes = ['text', 'text']
        col_model = [model.NAME_COLUMN,
                     model.VALUE_COLUMN]

        col = Gtk.TreeViewColumn()
        for i in range(2):
            col.pack_start(col_renders[i], False)
            col.add_attribute(col_renders[i],
                              col_attributes[i],
                              col_model[i])

        col.set_sizing(Gtk.TreeViewColumnSizing.AUTOSIZE)
        self.insert_column(col, -1)
        self.set_size_request(self.METADATA_MIN_WIDTH,
                              self.METADATA_MIN_HEIGHT)

if __name__ == '__main__':
    main_window = MainWindow()
    main_window.run()
