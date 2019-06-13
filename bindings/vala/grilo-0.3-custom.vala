namespace Grl {

	public struct MetadataKey {
		[CCode (cname ="GRL_METADATA_KEY_INVALID")]
		public static Grl.KeyID INVALID;
		[CCode (cname ="GRL_METADATA_KEY_ALBUM")]
		public static Grl.KeyID ALBUM;
		[CCode (cname ="GRL_METADATA_KEY_ARTIST")]
		public static Grl.KeyID ARTIST;
		[CCode (cname ="GRL_METADATA_KEY_AUTHOR")]
		public static Grl.KeyID AUTHOR;
		[CCode (cname ="GRL_METADATA_KEY_BITRATE")]
		public static Grl.KeyID BITRATE;
		[CCode (cname ="GRL_METADATA_KEY_CERTIFICATE")]
		public static Grl.KeyID CERTIFICATE;
		[CCode (cname ="GRL_METADATA_KEY_CHILDCOUNT")]
		public static Grl.KeyID CHILDCOUNT;
		[CCode (cname ="GRL_METADATA_KEY_PUBLICATION_DATE")]
		public static Grl.KeyID PUBLICATION_DATE;
		[CCode (cname ="GRL_METADATA_KEY_DESCRIPTION")]
		public static Grl.KeyID DESCRIPTION;
		[CCode (cname ="GRL_METADATA_KEY_DURATION")]
		public static Grl.KeyID DURATION;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_PLAYER")]
		public static Grl.KeyID EXTERNAL_PLAYER;
		[CCode (cname ="GRL_METADATA_KEY_EXTERNAL_URL")]
		public static Grl.KeyID EXTERNAL_URL;
		[CCode (cname ="GRL_METADATA_KEY_FRAMERATE")]
		public static Grl.KeyID FRAMERATE;
		[CCode (cname ="GRL_METADATA_KEY_GENRE")]
		public static Grl.KeyID GENRE;
		[CCode (cname ="GRL_METADATA_KEY_HEIGHT")]
		public static Grl.KeyID HEIGHT;
		[CCode (cname ="GRL_METADATA_KEY_ID")]
		public static Grl.KeyID ID;
		[CCode (cname ="GRL_METADATA_KEY_LAST_PLAYED")]
		public static Grl.KeyID LAST_PLAYED;
		[CCode (cname ="GRL_METADATA_KEY_LAST_POSITION")]
		public static Grl.KeyID LAST_POSITION;
		[CCode (cname ="GRL_METADATA_KEY_LICENSE")]
		public static Grl.KeyID LICENSE;
		[CCode (cname ="GRL_METADATA_KEY_LYRICS")]
		public static Grl.KeyID LYRICS;
		[CCode (cname ="GRL_METADATA_KEY_MIME")]
		public static Grl.KeyID MIME;
		[CCode (cname ="GRL_METADATA_KEY_PLAY_COUNT")]
		public static Grl.KeyID PLAY_COUNT;
		[CCode (cname ="GRL_METADATA_KEY_RATING")]
		public static Grl.KeyID RATING;
		[CCode (cname ="GRL_METADATA_KEY_REGION")]
		public static Grl.KeyID REGION;
		[CCode (cname ="GRL_METADATA_KEY_SITE")]
		public static Grl.KeyID SITE;
		[CCode (cname ="GRL_METADATA_KEY_SOURCE")]
		public static Grl.KeyID SOURCE;
		[CCode (cname ="GRL_METADATA_KEY_STUDIO")]
		public static Grl.KeyID STUDIO;
		[CCode (cname ="GRL_METADATA_KEY_THUMBNAIL")]
		public static Grl.KeyID THUMBNAIL;
		[CCode (cname ="GRL_METADATA_KEY_THUMBNAIL_BINARY")]
		public static Grl.KeyID THUMBNAIL_BINARY;
		[CCode (cname ="GRL_METADATA_KEY_TITLE")]
		public static Grl.KeyID TITLE;
		[CCode (cname ="GRL_METADATA_KEY_URL")]
		public static Grl.KeyID URL;
		[CCode (cname ="GRL_METADATA_KEY_WIDTH")]
		public static Grl.KeyID WIDTH;
		[CCode (cname ="GRL_METADATA_KEY_SEASON")]
		public static Grl.KeyID SEASON;
		[CCode (cname ="GRL_METADATA_KEY_EPISODE")]
		public static Grl.KeyID EPISODE;
		[CCode (cname ="GRL_METADATA_KEY_EPISODE_TITLE")]
		public static Grl.KeyID EPISODE_TITLE;
		[CCode (cname ="GRL_METADATA_KEY_SHOW")]
		public static Grl.KeyID SHOW;
		[CCode (cname ="GRL_METADATA_KEY_CREATION_DATE")]
		public static Grl.KeyID CREATION_DATE;
		[CCode (cname ="GRL_METADATA_KEY_CAMERA_MODEL")]
		public static Grl.KeyID CAMERA_MODEL;
		[CCode (cname ="GRL_METADATA_KEY_ORIENTATION")]
		public static Grl.KeyID ORIENTATION;
		[CCode (cname ="GRL_METADATA_KEY_FLASH_USED")]
		public static Grl.KeyID FLASH_USED;
		[CCode (cname ="GRL_METADATA_KEY_EXPOSURE_TIME")]
		public static Grl.KeyID EXPOSURE_TIME;
		[CCode (cname ="GRL_METADATA_KEY_ISO_SPEED")]
		public static Grl.KeyID ISO_SPEED;
		[CCode (cname ="GRL_METADATA_KEY_TRACK_NUMBER")]
		public static Grl.KeyID TRACK_NUMBER;
		[CCode (cname ="GRL_METADATA_KEY_MODIFICATION_DATE")]
		public static Grl.KeyID MODIFICATION_DATE;
		[CCode (cname ="GRL_METADATA_KEY_START_TIME")]
		public static Grl.KeyID START_TIME;
		[CCode (cname ="GRL_METADATA_KEY_KEYWORD")]
		public static Grl.KeyID KEYWORD;
		[CCode (cname ="GRL_METADATA_KEY_PERFORMER")]
		public static Grl.KeyID PERFORMER;
		[CCode (cname ="GRL_METADATA_KEY_PRODUCER")]
		public static Grl.KeyID PRODUCER;
		[CCode (cname ="GRL_METADATA_KEY_DIRECTOR")]
		public static Grl.KeyID DIRECTOR;
		[CCode (cname ="GRL_METADATA_KEY_ORIGINAL_TITLE")]
		public static Grl.KeyID ORIGINAL_TITLE;
		[CCode (cname ="GRL_METADATA_KEY_MB_ALBUM_ID")]
		public static Grl.KeyID MB_ALBUM_ID;
		[CCode (cname ="GRL_METADATA_KEY_MB_TRACK_ID")]
		public static Grl.KeyID MB_TRACK_ID;
		[CCode (cname ="GRL_METADATA_KEY_MB_ARTIST_ID")]
		public static Grl.KeyID MB_ARTIST_ID;
		[CCode (cname ="GRL_METADATA_KEY_MB_RECORDING_ID")]
		public static Grl.KeyID MB_RECORDING_ID;
		[CCode (cname ="GRL_METADATA_KEY_MB_RELEASE_ID")]
		public static Grl.KeyID MB_RELEASE_ID;
		[CCode (cname ="GRL_METADATA_KEY_MB_RELEASE_GROUP_ID")]
		public static Grl.KeyID MB_RELEASE_GROUP_ID;
		[CCode (cname ="GRL_METADATA_KEY_AUDIO_TRACK")]
		public static Grl.KeyID AUDIO_TRACK;
		[CCode (cname ="GRL_METADATA_KEY_ALBUM_DISC_NUMBER")]
		public static Grl.KeyID ALBUM_DISC_NUMBER;
		[CCode (cname ="GRL_METADATA_KEY_COMPOSER")]
		public static Grl.KeyID COMPOSER;
		[CCode (cname ="GRL_METADATA_KEY_ALBUM_ARTIST")]
		public static Grl.KeyID ALBUM_ARTIST;

		[CCode (cname ="GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN")]
		public static uint CHILDCOUNT_UNKNOWN;

		[CCode (cname ="GRL_SOURCE_REMAINING_UNKNOWN")]
		public static uint REMAINING_UNKNOWN;

		public static GLib.List list_new (Grl.KeyID p, ...);
	}
}
