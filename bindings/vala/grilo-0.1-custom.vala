namespace Grl {
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

	[Compact]
	public class MetadataKey {
		[CCode (cname ="GRL_METADATA_KEY_ALBUM")]
		public GLib.ParamSpec ALBUM;
		[CCode (cname ="GRL_METADATA_KEY_ARTIST")]
		public GLib.ParamSpec ARTIST;
		[CCode (cname ="GRL_METADATA_KEY_AUTHOR")]
		public GLib.ParamSpec AUTHOR;
		[CCode (cname ="GRL_METADATA_KEY_BITRATE")]
		public GLib.ParamSpec BITRATE;
		[CCode (cname ="GRL_METADATA_KEY_CERTIFICATE")]
		public GLib.ParamSpec CERTIFICATE;
		[CCode (cname ="GRL_METADATA_KEY_CHILDCOUNT")]
		public GLib.ParamSpec CHILDCOUNT;
		[CCode (cname ="GRL_METADATA_KEY_DATE")]
		public GLib.ParamSpec DATE;
		[CCode (cname ="GRL_METADATA_KEY_DESCRIPTION")]
		public GLib.ParamSpec DESCRIPTION;
		[CCode (cname ="GRL_METADATA_KEY_DURATION")]
		public GLib.ParamSpec DURATION;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_PLAYER")]
		public GLib.ParamSpec EXTERNAL_PLAYER;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_URL")]
		public GLib.ParamSpec EXTERNAL_URL;
		[CCode (cname ="GRL_METADATA_KEY_FRAMERATE")]
		public GLib.ParamSpec FRAMERATE;
		[CCode (cname ="GRL_METADATA_KEY_GENRE")]
		public GLib.ParamSpec GENRE;
		[CCode (cname ="GRL_METADATA_KEY_HEIGHT")]
		public GLib.ParamSpec HEIGHT;
		[CCode (cname ="GRL_METADATA_KEY_ID")]
		public static GLib.ParamSpec ID;
		[CCode (cname ="GRL_METADATA_KEY_LAST_PLAYED")]
		public GLib.ParamSpec LAST_PLAYED;
		[CCode (cname ="GRL_METADATA_KEY_LAST_POSITION")]
		public GLib.ParamSpec LAST_POSITION;
		[CCode (cname ="GRL_METADATA_KEY_LICENSE")]
		public GLib.ParamSpec LICENSE;
		[CCode (cname ="GRL_METADATA_KEY_LYRICS")]
		public GLib.ParamSpec LYRICS;
		[CCode (cname ="GRL_METADATA_KEY_MIME")]
		public GLib.ParamSpec MIME;
		[CCode (cname ="GRL_METADATA_KEY_PLAY_COUNT")]
		public GLib.ParamSpec PLAY_COUNT;
		[CCode (cname ="GRL_METADATA_KEY_RATING")]
		public GLib.ParamSpec RATING;
		[CCode (cname ="GRL_METADATA_KEY_SITE")]
		public GLib.ParamSpec SITE;
		[CCode (cname ="GRL_METADATA_KEY_SOURCE")]
		public GLib.ParamSpec SOURCE;
		[CCode (cname ="GRL_METADATA_KEY_STUDIO")]
		public GLib.ParamSpec STUDIO;
		[CCode (cname ="GRL_METADATA_KEY_THUMBNAIL")]
		public GLib.ParamSpec THUMBNAIL;
		[CCode (cname ="GRL_METADATA_KEY_TITLE")]
		public static GLib.ParamSpec TITLE;
		[CCode (cname ="GRL_METADATA_KEY_URL")]
		public static GLib.ParamSpec URL;
		[CCode (cname ="GRL_METADATA_KEY_WIDTH")]
		public GLib.ParamSpec WIDTH;

		public static unowned GLib.List list_new (GLib.ParamSpec p, ...);
	}
}
