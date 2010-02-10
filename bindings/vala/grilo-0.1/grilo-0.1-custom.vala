namespace Grl {
	public class ContentMedia {
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
	}

	[CCode (instance_pos = 2.1)]
	public delegate void MediaSourceMetadataCb (Grl.MediaSource source, Grl.ContentMedia? media, GLib.Error error);
	[CCode (instance_pos = 2.1)]
	public delegate void MediaSourceRemoveCb (Grl.MediaSource source, Grl.ContentMedia? media, GLib.Error error);
	[CCode (instance_pos = 4.1)]
	public delegate void MediaSourceResultCb (Grl.MediaSource source, uint browse_id, Grl.ContentMedia? media, uint remaining, GLib.Error? error);
	[CCode (instance_pos = 4.1)]
	public delegate void MediaSourceStoreCb (Grl.MediaSource source, Grl.ContentBox parent, Grl.ContentMedia? media, GLib.Error error);
	[CCode (instance_pos = 2.1)]
	public delegate void MetadataSourceResolveCb (Grl.MetadataSource source, Grl.ContentMedia? media, GLib.Error error);
}
