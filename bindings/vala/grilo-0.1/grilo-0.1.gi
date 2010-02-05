<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Grl">
		<function name="log_init" symbol="grl_log_init">
			<return-type type="void"/>
			<parameters>
				<parameter name="domains" type="gchar*"/>
			</parameters>
		</function>
		<callback name="GrlMediaSourceMetadataCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="GrlMediaSource*"/>
				<parameter name="media" type="GrlContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="GrlMediaSourceResultCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="GrlMediaSource*"/>
				<parameter name="browse_id" type="guint"/>
				<parameter name="media" type="GrlContentMedia*"/>
				<parameter name="remaining" type="guint"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="GrlMediaSourceStoreCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="GrlMediaSource*"/>
				<parameter name="parent" type="GrlContentBox*"/>
				<parameter name="media" type="GrlContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="GrlMetadataSourceResolveCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="GrlMetadataSource*"/>
				<parameter name="media" type="GrlContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<struct name="GrlKeyID">
		</struct>
		<struct name="GrlMediaSourceBrowseSpec">
			<field name="source" type="GrlMediaSource*"/>
			<field name="browse_id" type="guint"/>
			<field name="container" type="GrlContentMedia*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="GrlMetadataResolutionFlags"/>
			<field name="callback" type="GrlMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlMediaSourceMetadataSpec">
			<field name="source" type="GrlMediaSource*"/>
			<field name="media" type="GrlContentMedia*"/>
			<field name="keys" type="GList*"/>
			<field name="flags" type="GrlMetadataResolutionFlags"/>
			<field name="callback" type="GrlMediaSourceMetadataCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlMediaSourceQuerySpec">
			<field name="source" type="GrlMediaSource*"/>
			<field name="query_id" type="guint"/>
			<field name="query" type="gchar*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="GrlMetadataResolutionFlags"/>
			<field name="callback" type="GrlMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlMediaSourceSearchSpec">
			<field name="source" type="GrlMediaSource*"/>
			<field name="search_id" type="guint"/>
			<field name="text" type="gchar*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="GrlMetadataResolutionFlags"/>
			<field name="callback" type="GrlMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlMediaSourceStoreSpec">
			<field name="source" type="GrlMediaSource*"/>
			<field name="parent" type="GrlContentBox*"/>
			<field name="media" type="GrlContentMedia*"/>
			<field name="callback" type="GrlMediaSourceStoreCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlMetadataKey">
			<method name="list_new" symbol="grl_metadata_key_list_new">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="first_key" type="GrlKeyID"/>
				</parameters>
			</method>
			<field name="id" type="GrlKeyID"/>
			<field name="name" type="gchar*"/>
			<field name="desc" type="gchar*"/>
		</struct>
		<struct name="GrlMetadataSourceResolveSpec">
			<field name="source" type="GrlMetadataSource*"/>
			<field name="keys" type="GList*"/>
			<field name="media" type="GrlContentMedia*"/>
			<field name="flags" type="guint"/>
			<field name="callback" type="GrlMetadataSourceResolveCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="GrlPluginDescriptor">
			<field name="info" type="GrlPluginInfo"/>
			<field name="plugin_init" type="GCallback"/>
			<field name="plugin_deinit" type="GCallback"/>
		</struct>
		<struct name="GrlPluginInfo">
			<field name="id" type="gchar*"/>
			<field name="name" type="gchar*"/>
			<field name="desc" type="gchar*"/>
			<field name="version" type="gchar*"/>
			<field name="author" type="gchar*"/>
			<field name="license" type="gchar*"/>
			<field name="site" type="gchar*"/>
		</struct>
		<enum name="GrlMetadataResolutionFlags">
			<member name="GRL_RESOLVE_NORMAL" value="0"/>
			<member name="GRL_RESOLVE_FULL" value="1"/>
			<member name="GRL_RESOLVE_IDLE_RELAY" value="2"/>
			<member name="GRL_RESOLVE_FAST_ONLY" value="4"/>
		</enum>
		<enum name="GrlSupportedOps">
			<member name="GRL_OP_NONE" value="0"/>
			<member name="GRL_OP_METADATA" value="1"/>
			<member name="GRL_OP_RESOLVE" value="2"/>
			<member name="GRL_OP_BROWSE" value="4"/>
			<member name="GRL_OP_SEARCH" value="8"/>
			<member name="GRL_OP_QUERY" value="16"/>
			<member name="GRL_OP_STORE" value="32"/>
			<member name="GRL_OP_STORE_PARENT" value="64"/>
		</enum>
		<object name="GrlContent" parent="GObject" type-name="GrlContent" get-type="grl_content_get_type">
			<method name="add" symbol="grl_content_add">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="get" symbol="grl_content_get">
				<return-type type="GValue*"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="get_float" symbol="grl_content_get_float">
				<return-type type="gfloat"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="get_int" symbol="grl_content_get_int">
				<return-type type="gint"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="get_keys" symbol="grl_content_get_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
				</parameters>
			</method>
			<method name="get_overwrite" symbol="grl_content_get_overwrite">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
				</parameters>
			</method>
			<method name="get_string" symbol="grl_content_get_string">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="has_key" symbol="grl_content_has_key">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="key_is_known" symbol="grl_content_key_is_known">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<constructor name="new" symbol="grl_content_new">
				<return-type type="GrlContent*"/>
			</constructor>
			<method name="remove" symbol="grl_content_remove">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="set" symbol="grl_content_set">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<method name="set_float" symbol="grl_content_set_float">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
					<parameter name="floatvalue" type="gint"/>
				</parameters>
			</method>
			<method name="set_int" symbol="grl_content_set_int">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
					<parameter name="intvalue" type="gint"/>
				</parameters>
			</method>
			<method name="set_overwrite" symbol="grl_content_set_overwrite">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="overwrite" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_string" symbol="grl_content_set_string">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContent*"/>
					<parameter name="key" type="GrlKeyID"/>
					<parameter name="strvalue" type="gchar*"/>
				</parameters>
			</method>
			<property name="overwrite" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="GrlContentAudio" parent="GrlContentMedia" type-name="GrlContentAudio" get-type="grl_content_audio_get_type">
			<constructor name="new" symbol="grl_content_audio_new">
				<return-type type="GrlContentMedia*"/>
			</constructor>
		</object>
		<object name="GrlContentBox" parent="GrlContentMedia" type-name="GrlContentBox" get-type="grl_content_box_get_type">
			<method name="get_childcount" symbol="grl_content_box_get_childcount">
				<return-type type="gint"/>
				<parameters>
					<parameter name="content" type="GrlContentBox*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="grl_content_box_new">
				<return-type type="GrlContentMedia*"/>
			</constructor>
			<method name="set_childcount" symbol="grl_content_box_set_childcount">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContentBox*"/>
					<parameter name="childcount" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="GrlContentImage" parent="GrlContentMedia" type-name="GrlContentImage" get-type="grl_content_image_get_type">
			<constructor name="new" symbol="grl_content_image_new">
				<return-type type="GrlContentMedia*"/>
			</constructor>
			<method name="set_size" symbol="grl_content_image_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContentImage*"/>
					<parameter name="width" type="gint"/>
					<parameter name="height" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="GrlContentMedia" parent="GrlContent" type-name="GrlContentMedia" get-type="grl_content_media_get_type">
			<constructor name="new" symbol="grl_content_media_new">
				<return-type type="GrlContentMedia*"/>
			</constructor>
			<method name="set_rating" symbol="grl_content_media_set_rating">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContentMedia*"/>
					<parameter name="rating" type="gchar*"/>
					<parameter name="max" type="gchar*"/>
				</parameters>
			</method>
		</object>
		<object name="GrlContentVideo" parent="GrlContentMedia" type-name="GrlContentVideo" get-type="grl_content_video_get_type">
			<constructor name="new" symbol="grl_content_video_new">
				<return-type type="GrlContentMedia*"/>
			</constructor>
			<method name="set_size" symbol="grl_content_video_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="GrlContentVideo*"/>
					<parameter name="width" type="gint"/>
					<parameter name="height" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="GrlMediaPlugin" parent="GObject" type-name="GrlMediaPlugin" get-type="grl_media_plugin_get_type">
			<method name="get_author" symbol="grl_media_plugin_get_author">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_description" symbol="grl_media_plugin_get_description">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_id" symbol="grl_media_plugin_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_license" symbol="grl_media_plugin_get_license">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="grl_media_plugin_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_site" symbol="grl_media_plugin_get_site">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_version" symbol="grl_media_plugin_get_version">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
		</object>
		<object name="GrlMediaSource" parent="GrlMetadataSource" type-name="GrlMediaSource" get-type="grl_media_source_get_type">
			<method name="browse" symbol="grl_media_source_browse">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="container" type="GrlContentMedia*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="GrlMetadataResolutionFlags"/>
					<parameter name="callback" type="GrlMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="cancel" symbol="grl_media_source_cancel">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</method>
			<method name="get_auto_split_threshold" symbol="grl_media_source_get_auto_split_threshold">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
				</parameters>
			</method>
			<method name="get_operation_data" symbol="grl_media_source_get_operation_data">
				<return-type type="gpointer"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</method>
			<method name="metadata" symbol="grl_media_source_metadata">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="media" type="GrlContentMedia*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="flags" type="GrlMetadataResolutionFlags"/>
					<parameter name="callback" type="GrlMediaSourceMetadataCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="query" symbol="grl_media_source_query">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="query" type="gchar*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="GrlMetadataResolutionFlags"/>
					<parameter name="callback" type="GrlMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="search" symbol="grl_media_source_search">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="text" type="gchar*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="GrlMetadataResolutionFlags"/>
					<parameter name="callback" type="GrlMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="set_auto_split_threshold" symbol="grl_media_source_set_auto_split_threshold">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="threshold" type="guint"/>
				</parameters>
			</method>
			<method name="set_operation_data" symbol="grl_media_source_set_operation_data">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
					<parameter name="data" type="gpointer"/>
				</parameters>
			</method>
			<method name="store" symbol="grl_media_source_store">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="parent" type="GrlContentBox*"/>
					<parameter name="media" type="GrlContentMedia*"/>
					<parameter name="callback" type="GrlMediaSourceStoreCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<property name="auto-split-threshold" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<vfunc name="browse">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="bs" type="GrlMediaSourceBrowseSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="cancel">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</vfunc>
			<vfunc name="metadata">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="grl" type="GrlMediaSourceMetadataSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="query">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="qs" type="GrlMediaSourceQuerySpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="search">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="ss" type="GrlMediaSourceSearchSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="store">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMediaSource*"/>
					<parameter name="ss" type="GrlMediaSourceStoreSpec*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="GrlMetadataSource" parent="GrlMediaPlugin" type-name="GrlMetadataSource" get-type="grl_metadata_source_get_type">
			<method name="filter_slow" symbol="grl_metadata_source_filter_slow">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="keys" type="GList**"/>
					<parameter name="return_filtered" type="gboolean"/>
				</parameters>
			</method>
			<method name="filter_supported" symbol="grl_metadata_source_filter_supported">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="keys" type="GList**"/>
					<parameter name="return_filtered" type="gboolean"/>
				</parameters>
			</method>
			<method name="get_description" symbol="grl_metadata_source_get_description">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<method name="get_id" symbol="grl_metadata_source_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="grl_metadata_source_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<method name="key_depends" symbol="grl_metadata_source_key_depends">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="key_id" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="resolve" symbol="grl_metadata_source_resolve">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="media" type="GrlContentMedia*"/>
					<parameter name="flags" type="guint"/>
					<parameter name="callback" type="GrlMetadataSourceResolveCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="slow_keys" symbol="grl_metadata_source_slow_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<method name="supported_keys" symbol="grl_metadata_source_supported_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<method name="supported_operations" symbol="grl_metadata_source_supported_operations">
				<return-type type="GrlSupportedOps"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</method>
			<property name="source-desc" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="source-id" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="source-name" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<vfunc name="key_depends">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="key_id" type="GrlKeyID"/>
				</parameters>
			</vfunc>
			<vfunc name="resolve">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
					<parameter name="rs" type="GrlMetadataSourceResolveSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="slow_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="supported_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="supported_operations">
				<return-type type="GrlSupportedOps"/>
				<parameters>
					<parameter name="source" type="GrlMetadataSource*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="GrlPluginRegistry" parent="GObject" type-name="GrlPluginRegistry" get-type="grl_plugin_registry_get_type">
			<method name="get_instance" symbol="grl_plugin_registry_get_instance">
				<return-type type="GrlPluginRegistry*"/>
			</method>
			<method name="get_sources" symbol="grl_plugin_registry_get_sources">
				<return-type type="GrlMediaPlugin**"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
				</parameters>
			</method>
			<method name="load" symbol="grl_plugin_registry_load">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<method name="load_all" symbol="grl_plugin_registry_load_all">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
				</parameters>
			</method>
			<method name="load_directory" symbol="grl_plugin_registry_load_directory">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<method name="lookup_metadata_key" symbol="grl_plugin_registry_lookup_metadata_key">
				<return-type type="GrlMetadataKey*"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="key_id" type="GrlKeyID"/>
				</parameters>
			</method>
			<method name="lookup_source" symbol="grl_plugin_registry_lookup_source">
				<return-type type="GrlMediaPlugin*"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="source_id" type="gchar*"/>
				</parameters>
			</method>
			<method name="register_source" symbol="grl_plugin_registry_register_source">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="plugin" type="GrlPluginInfo*"/>
					<parameter name="source" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<method name="unload" symbol="grl_plugin_registry_unload">
				<return-type type="void"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="plugin_id" type="gchar*"/>
				</parameters>
			</method>
			<method name="unregister_source" symbol="grl_plugin_registry_unregister_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="registry" type="GrlPluginRegistry*"/>
					<parameter name="source" type="GrlMediaPlugin*"/>
				</parameters>
			</method>
			<signal name="source-added" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="GrlPluginRegistry*"/>
					<parameter name="p0" type="GrlMediaPlugin*"/>
				</parameters>
			</signal>
			<signal name="source-removed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="GrlPluginRegistry*"/>
					<parameter name="p0" type="GrlMediaPlugin*"/>
				</parameters>
			</signal>
		</object>
		<constant name="GRL_KEYID_FORMAT" type="char*" value="u"/>
		<constant name="GRL_METADATA_KEY_ALBUM" type="int" value="4"/>
		<constant name="GRL_METADATA_KEY_ALBUM_DESC" type="char*" value="Album the media belongs to"/>
		<constant name="GRL_METADATA_KEY_ALBUM_NAME" type="char*" value="album"/>
		<constant name="GRL_METADATA_KEY_ARTIST" type="int" value="3"/>
		<constant name="GRL_METADATA_KEY_ARTIST_DESC" type="char*" value="Main artist"/>
		<constant name="GRL_METADATA_KEY_ARTIST_NAME" type="char*" value="artist"/>
		<constant name="GRL_METADATA_KEY_AUTHOR" type="int" value="8"/>
		<constant name="GRL_METADATA_KEY_AUTHOR_DESC" type="char*" value="Creator of the media"/>
		<constant name="GRL_METADATA_KEY_AUTHOR_NAME" type="char*" value="author"/>
		<constant name="GRL_METADATA_KEY_CHILDCOUNT" type="int" value="15"/>
		<constant name="GRL_METADATA_KEY_CHILDCOUNT_DESC" type="char*" value="Number of items contained in a container"/>
		<constant name="GRL_METADATA_KEY_CHILDCOUNT_NAME" type="char*" value="childcount"/>
		<constant name="GRL_METADATA_KEY_CHILDCOUNT_UNKNOWN" type="int" value="-1"/>
		<constant name="GRL_METADATA_KEY_DATE" type="int" value="14"/>
		<constant name="GRL_METADATA_KEY_DATE_DESC" type="char*" value="Publishing or recording date"/>
		<constant name="GRL_METADATA_KEY_DATE_NAME" type="char*" value="date"/>
		<constant name="GRL_METADATA_KEY_DESCRIPTION" type="int" value="9"/>
		<constant name="GRL_METADATA_KEY_DESCRIPTION_DESC" type="char*" value="Description of the media"/>
		<constant name="GRL_METADATA_KEY_DESCRIPTION_NAME" type="char*" value="description"/>
		<constant name="GRL_METADATA_KEY_DURATION" type="int" value="13"/>
		<constant name="GRL_METADATA_KEY_DURATION_DESC" type="char*" value="Media duration"/>
		<constant name="GRL_METADATA_KEY_DURATION_NAME" type="char*" value="duration"/>
		<constant name="GRL_METADATA_KEY_FRAMERATE" type="int" value="19"/>
		<constant name="GRL_METADATA_KEY_FRAMERATE_DESC" type="char*" value="Frames per second"/>
		<constant name="GRL_METADATA_KEY_FRAMERATE_NAME" type="char*" value="framerate"/>
		<constant name="GRL_METADATA_KEY_GENRE" type="int" value="5"/>
		<constant name="GRL_METADATA_KEY_GENRE_DESC" type="char*" value="Genre of the media"/>
		<constant name="GRL_METADATA_KEY_GENRE_NAME" type="char*" value="genre"/>
		<constant name="GRL_METADATA_KEY_HEIGHT" type="int" value="18"/>
		<constant name="GRL_METADATA_KEY_HEIGHT_DESC" type="char*" value="height of video (y-axis resolution)"/>
		<constant name="GRL_METADATA_KEY_HEIGHT_NAME" type="char*" value="height"/>
		<constant name="GRL_METADATA_KEY_ID" type="int" value="7"/>
		<constant name="GRL_METADATA_KEY_ID_DESC" type="char*" value="Identifier of media"/>
		<constant name="GRL_METADATA_KEY_ID_NAME" type="char*" value="id"/>
		<constant name="GRL_METADATA_KEY_LYRICS" type="int" value="11"/>
		<constant name="GRL_METADATA_KEY_LYRICS_DESC" type="char*" value="Song lyrics"/>
		<constant name="GRL_METADATA_KEY_LYRICS_NAME" type="char*" value="lyrics"/>
		<constant name="GRL_METADATA_KEY_MIME" type="int" value="16"/>
		<constant name="GRL_METADATA_KEY_MIME_DESC" type="char*" value="Media mime type"/>
		<constant name="GRL_METADATA_KEY_MIME_NAME" type="char*" value="mime-type"/>
		<constant name="GRL_METADATA_KEY_RATING" type="int" value="20"/>
		<constant name="GRL_METADATA_KEY_RATING_DESC" type="char*" value="Media rating"/>
		<constant name="GRL_METADATA_KEY_RATING_NAME" type="char*" value="rating"/>
		<constant name="GRL_METADATA_KEY_SITE" type="int" value="12"/>
		<constant name="GRL_METADATA_KEY_SITE_DESC" type="char*" value="Site"/>
		<constant name="GRL_METADATA_KEY_SITE_NAME" type="char*" value="site"/>
		<constant name="GRL_METADATA_KEY_SOURCE" type="int" value="10"/>
		<constant name="GRL_METADATA_KEY_SOURCE_DESC" type="char*" value="Source ID providing the content"/>
		<constant name="GRL_METADATA_KEY_SOURCE_NAME" type="char*" value="source"/>
		<constant name="GRL_METADATA_KEY_THUMBNAIL" type="int" value="6"/>
		<constant name="GRL_METADATA_KEY_THUMBNAIL_DESC" type="char*" value="Thumbnail image"/>
		<constant name="GRL_METADATA_KEY_THUMBNAIL_NAME" type="char*" value="thumbnail"/>
		<constant name="GRL_METADATA_KEY_TITLE" type="int" value="1"/>
		<constant name="GRL_METADATA_KEY_TITLE_DESC" type="char*" value="Title of the media"/>
		<constant name="GRL_METADATA_KEY_TITLE_NAME" type="char*" value="title"/>
		<constant name="GRL_METADATA_KEY_URL" type="int" value="2"/>
		<constant name="GRL_METADATA_KEY_URL_DESC" type="char*" value="Media URL"/>
		<constant name="GRL_METADATA_KEY_URL_NAME" type="char*" value="url"/>
		<constant name="GRL_METADATA_KEY_WIDTH" type="int" value="17"/>
		<constant name="GRL_METADATA_KEY_WIDTH_DESC" type="char*" value="Width of video (x-axis resolution)"/>
		<constant name="GRL_METADATA_KEY_WIDTH_NAME" type="char*" value="width"/>
		<constant name="GRL_PLUGIN_PATH_VAR" type="char*" value="GRL_PLUGIN_PATH"/>
	</namespace>
</api>
