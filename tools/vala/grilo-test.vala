using Grl;

public class SimplePlaylist : Object {
	private GLib.List<MediaSource> source_list;
	MainLoop main_loop = new MainLoop (null, false);
	int processed_sources = 0;

	construct {
		Grl.log_init ("*:-");
		var registry = Grl.PluginRegistry.get_default ();

		registry.source_added.connect (source_added_cb);
		registry.source_removed.connect (source_removed_cb);

		if (registry.load_all () == false) {
			error ("Failed to load plugins.");
		}

	}

	public void source_added_cb (MediaPlugin plugin) {
		var source = plugin as MetadataSource;

		var ops = source.supported_operations ();
		if ((ops & Grl.SupportedOps.SEARCH) != 0) {
			debug ("Detected new source availabe: '%s' and it supports search", source.get_name ());
			source_list.append (source as MediaSource);
			debug ("source list size = %u", source_list.length ());
		}
	}

	public void source_removed_cb (MediaPlugin source) {
		debug ("Source '%s' is gone", (source as MetadataSource).get_name ());
	}

	public SimplePlaylist () {
	}

	private void search_cb (Grl.MediaSource source,
							uint browse_id,
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
		unowned GLib.List keys = Grl.MetadataKey.list_new (Grl.MetadataKey.ID, Grl.MetadataKey.TITLE, Grl.MetadataKey.URL);

		foreach (MediaSource source in source_list) {
			debug ("%s - %s", source.get_name (), q);
			source.search (q, keys, 0, 100, Grl.MetadataResolutionFlags.FULL | Grl.MetadataResolutionFlags.IDLE_RELAY, search_cb);
		}
	}

	public void run () {
		main_loop.run ();
	}

	public static int main (string[] args) {
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

