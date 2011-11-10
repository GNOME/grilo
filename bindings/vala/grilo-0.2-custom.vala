namespace Grl {
	[CCode (instance_pos = 3.1)]
	public delegate void MediaSourceMetadataCb (Grl.MediaSource source, uint operation_id, Grl.Media? media, GLib.Error error);
	[CCode (instance_pos = 2.1)]
	public delegate void MediaSourceRemoveCb (Grl.MediaSource source, Grl.Media? media, GLib.Error error);
	[CCode (instance_pos = 4.1)]
	public delegate void MediaSourceResultCb (Grl.MediaSource source, uint operation_id, Grl.Media? media, uint remaining, GLib.Error? error);
	[CCode (instance_pos = 3.1)]
	public delegate void MediaSourceStoreCb (Grl.MediaSource source, Grl.MediaBox? parent, Grl.Media? media, GLib.Error? error);
	[CCode (instance_pos = 3.1)]
	public delegate void MetadataSourceResolveCb (Grl.MetadataSource source, uint operation_id, Grl.Media? media, GLib.Error? error);
	[CCode (instance_pos = 3.1)]
	public delegate void MetadataSourceSetMetadataCb (Grl.MetadataSource source, Grl.Media? media, GLib.List failed_keys, GLib.Error? error);

	[Compact]
	public class MetadataKey {
		[CCode (cname ="GRL_METADATA_KEY_ALBUM")]
		public static GLib.ParamSpec ALBUM;
		[CCode (cname ="GRL_METADATA_KEY_ARTIST")]
		public static GLib.ParamSpec ARTIST;
		[CCode (cname ="GRL_METADATA_KEY_AUTHOR")]
		public static GLib.ParamSpec AUTHOR;
		[CCode (cname ="GRL_METADATA_KEY_BITRATE")]
		public static GLib.ParamSpec BITRATE;
		[CCode (cname ="GRL_METADATA_KEY_CERTIFICATE")]
		public static GLib.ParamSpec CERTIFICATE;
		[CCode (cname ="GRL_METADATA_KEY_CHILDCOUNT")]
		public static GLib.ParamSpec CHILDCOUNT;
		[CCode (cname ="GRL_METADATA_KEY_DATE")]
		public static GLib.ParamSpec DATE;
		[CCode (cname ="GRL_METADATA_KEY_DESCRIPTION")]
		public static GLib.ParamSpec DESCRIPTION;
		[CCode (cname ="GRL_METADATA_KEY_DURATION")]
		public static GLib.ParamSpec DURATION;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_PLAYER")]
		public static GLib.ParamSpec EXTERNAL_PLAYER;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_URL")]
		public static GLib.ParamSpec EXTERNAL_URL;
		[CCode (cname ="GRL_METADATA_KEY_FRAMERATE")]
		public static GLib.ParamSpec FRAMERATE;
		[CCode (cname ="GRL_METADATA_KEY_GENRE")]
		public static GLib.ParamSpec GENRE;
		[CCode (cname ="GRL_METADATA_KEY_HEIGHT")]
		public static GLib.ParamSpec HEIGHT;
		[CCode (cname ="GRL_METADATA_KEY_ID")]
		public static GLib.ParamSpec ID;
		[CCode (cname ="GRL_METADATA_KEY_LAST_PLAYED")]
		public static GLib.ParamSpec LAST_PLAYED;
		[CCode (cname ="GRL_METADATA_KEY_LAST_POSITION")]
		public static GLib.ParamSpec LAST_POSITION;
		[CCode (cname ="GRL_METADATA_KEY_LICENSE")]
		public static GLib.ParamSpec LICENSE;
		[CCode (cname ="GRL_METADATA_KEY_LYRICS")]
		public static GLib.ParamSpec LYRICS;
		[CCode (cname ="GRL_METADATA_KEY_MIME")]
		public static GLib.ParamSpec MIME;
		[CCode (cname ="GRL_METADATA_KEY_PLAY_COUNT")]
		public static GLib.ParamSpec PLAY_COUNT;
		[CCode (cname ="GRL_METADATA_KEY_RATING")]
		public static GLib.ParamSpec RATING;
		[CCode (cname ="GRL_METADATA_KEY_SITE")]
		public static GLib.ParamSpec SITE;
		[CCode (cname ="GRL_METADATA_KEY_SOURCE")]
		public static GLib.ParamSpec SOURCE;
		[CCode (cname ="GRL_METADATA_KEY_STUDIO")]
		public static GLib.ParamSpec STUDIO;
		[CCode (cname ="GRL_METADATA_KEY_THUMBNAIL")]
		public static GLib.ParamSpec THUMBNAIL;
		[CCode (cname ="GRL_METADATA_KEY_TITLE")]
		public static GLib.ParamSpec TITLE;
		[CCode (cname ="GRL_METADATA_KEY_URL")]
		public static GLib.ParamSpec URL;
		[CCode (cname ="GRL_METADATA_KEY_WIDTH")]
		public static GLib.ParamSpec WIDTH;

		public static unowned GLib.List list_new (GLib.ParamSpec p, ...);
	}
}
