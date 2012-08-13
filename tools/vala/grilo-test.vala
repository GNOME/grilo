using Grl;

public class SimplePlaylist : Object {
	private GLib.List<Grl.Source> source_list;
	MainLoop main_loop = new MainLoop (null, false);
	int processed_sources = 0;

	construct {
		var registry = Grl.Registry.get_default ();

		registry.source_added.connect (source_added_cb);
		registry.source_removed.connect (source_removed_cb);

		if (registry.load_all_plugins () == false) {
			error ("Failed to load plugins.");
		}

	}

	public void source_added_cb (Grl.Source source) {
		var ops = source.supported_operations ();
		if ((ops & Grl.SupportedOps.SEARCH) != 0) {
			debug ("Detected new source availabe: '%s' and it supports search", source.get_name ());
			source_list.append (source as Grl.Source);
			debug ("source list size = %u", source_list.length ());
		}
	}

	public void source_removed_cb (Grl.Source source) {
		debug ("Source '%s' is gone", source.get_name ());
	}

	public SimplePlaylist () {
	}

	private void search_cb (Grl.Source source,
							uint operation_id,
							Grl.Media? media,
							uint remaining,
							GLib.Error? error) {
		if (error != null) {
			critical ("Error: %s", error.message);
		}

		if (media != null && (media is MediaAudio || media is MediaVideo)) {
                var url = media.get_url ();
                if (url != null) {
                        print ("%s\n", media.get_url ());
                }
		}

		if (remaining == 0) {
			processed_sources++;
			debug ("%s finished", source.get_name ());
		}

		debug ("processed sources %d - source list size %u", processed_sources, source_list.length ());
		if (processed_sources == source_list.length ()) {
			main_loop.quit ();
		}
	}

	public void search (string q) {
		unowned GLib.List keys = Grl.MetadataKey.list_new (MetadataKey.ID, MetadataKey.TITLE, MetadataKey.URL);
      Caps caps = null;
      OperationOptions options = new OperationOptions(caps);
      options.set_skip (0);
      options.set_count (100);
      options.set_flags (ResolutionFlags.FULL | ResolutionFlags.IDLE_RELAY);

		foreach (Grl.Source source in source_list) {
			debug ("%s - %s", source.get_name (), q);
			source.search (q, keys, options, search_cb);
		}
	}

	public void run () {
		main_loop.run ();
	}

	public static int main (string[] args) {
		Grl.init(ref args);
		var playlist = new SimplePlaylist ();

        if (args[1] != null) {
            playlist.search (args[1]);
            playlist.run ();
        } else {
            print ("Query parameter missing\n");
        }

		return 0;
	}
}

