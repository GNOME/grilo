namespace Grl {
	public class Media {
		public unowned string get_id ();
		public unowned string get_url ();
		public unowned string get_author ();
		public unowned string get_title ();
		public unowned string get_description ();
		public unowned string get_source ();
		public unowned string get_thumbnail ();
		public unowned string get_site ();
		public unowned string get_date ();
		public unowned string get_mime ();
		public unowned string get_rating ();
		public int get_duration ();

		public void set_id (string id);
		public void set_url (string url);
		public void set_author (string url);
		public void set_title (string title);
		public void set_description (string description);
		public void set_source (string source);
		public void set_thumbnail (string thumbnail);
		public void set_site (string site);
		public void set_duration (int duration);
		public void set_date (string date);
		public void set_mime (string mime);
	}

	[CCode (instance_pos = 2.1)]
	public delegate void MediaSourceMetadataCb (MediaSource source, Media? media, GLib.Error error);
	[CCode (instance_pos = 2.1)]
	public delegate void MediaSourceRemoveCb (MediaSource source, Media? media, GLib.Error error);
	[CCode (instance_pos = 4.1)]
	public delegate void MediaSourceResultCb (MediaSource source, uint browse_id, Media? media, uint remaining, GLib.Error? error);
	[CCode (instance_pos = 4.1)]
	public delegate void MediaSourceStoreCb (MediaSource source, MediaBox? parent, Media? media, GLib.Error? error);
	[CCode (instance_pos = 2.1)]
	public delegate void MetadataSourceResolveCb (MetadataSource source, Media? media, GLib.Error? error);
}
