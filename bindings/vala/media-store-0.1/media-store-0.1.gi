<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Ms">
		<function name="log_init" symbol="ms_log_init">
			<return-type type="void"/>
			<parameters>
				<parameter name="domains" type="gchar*"/>
			</parameters>
		</function>
		<callback name="MsMediaSourceMetadataCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="MsMediaSource*"/>
				<parameter name="media" type="MsContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="MsMediaSourceResultCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="MsMediaSource*"/>
				<parameter name="browse_id" type="guint"/>
				<parameter name="media" type="MsContentMedia*"/>
				<parameter name="remaining" type="guint"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="MsMediaSourceStoreCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="MsMediaSource*"/>
				<parameter name="parent" type="MsContentBox*"/>
				<parameter name="media" type="MsContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<callback name="MsMetadataSourceResolveCb">
			<return-type type="void"/>
			<parameters>
				<parameter name="source" type="MsMetadataSource*"/>
				<parameter name="media" type="MsContentMedia*"/>
				<parameter name="user_data" type="gpointer"/>
				<parameter name="error" type="GError*"/>
			</parameters>
		</callback>
		<struct name="MsKeyID">
		</struct>
		<struct name="MsMediaSourceBrowseSpec">
			<field name="source" type="MsMediaSource*"/>
			<field name="browse_id" type="guint"/>
			<field name="container" type="MsContentMedia*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="MsMetadataResolutionFlags"/>
			<field name="callback" type="MsMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsMediaSourceMetadataSpec">
			<field name="source" type="MsMediaSource*"/>
			<field name="media" type="MsContentMedia*"/>
			<field name="keys" type="GList*"/>
			<field name="flags" type="MsMetadataResolutionFlags"/>
			<field name="callback" type="MsMediaSourceMetadataCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsMediaSourceQuerySpec">
			<field name="source" type="MsMediaSource*"/>
			<field name="query_id" type="guint"/>
			<field name="query" type="gchar*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="MsMetadataResolutionFlags"/>
			<field name="callback" type="MsMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsMediaSourceSearchSpec">
			<field name="source" type="MsMediaSource*"/>
			<field name="search_id" type="guint"/>
			<field name="text" type="gchar*"/>
			<field name="keys" type="GList*"/>
			<field name="skip" type="guint"/>
			<field name="count" type="guint"/>
			<field name="flags" type="MsMetadataResolutionFlags"/>
			<field name="callback" type="MsMediaSourceResultCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsMediaSourceStoreSpec">
			<field name="source" type="MsMediaSource*"/>
			<field name="parent" type="MsContentBox*"/>
			<field name="media" type="MsContentMedia*"/>
			<field name="callback" type="MsMediaSourceStoreCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsMetadataKey">
			<method name="list_new" symbol="ms_metadata_key_list_new">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="first_key" type="MsKeyID"/>
				</parameters>
			</method>
			<field name="id" type="MsKeyID"/>
			<field name="name" type="gchar*"/>
			<field name="desc" type="gchar*"/>
		</struct>
		<struct name="MsMetadataSourceResolveSpec">
			<field name="source" type="MsMetadataSource*"/>
			<field name="keys" type="GList*"/>
			<field name="media" type="MsContentMedia*"/>
			<field name="flags" type="guint"/>
			<field name="callback" type="MsMetadataSourceResolveCb"/>
			<field name="user_data" type="gpointer"/>
		</struct>
		<struct name="MsPluginDescriptor">
			<field name="info" type="MsPluginInfo"/>
			<field name="plugin_init" type="GCallback"/>
			<field name="plugin_deinit" type="GCallback"/>
		</struct>
		<struct name="MsPluginInfo">
			<field name="id" type="gchar*"/>
			<field name="name" type="gchar*"/>
			<field name="desc" type="gchar*"/>
			<field name="version" type="gchar*"/>
			<field name="author" type="gchar*"/>
			<field name="license" type="gchar*"/>
			<field name="site" type="gchar*"/>
		</struct>
		<enum name="MsMetadataResolutionFlags">
			<member name="MS_RESOLVE_NORMAL" value="0"/>
			<member name="MS_RESOLVE_FULL" value="1"/>
			<member name="MS_RESOLVE_IDLE_RELAY" value="2"/>
			<member name="MS_RESOLVE_FAST_ONLY" value="4"/>
		</enum>
		<enum name="MsSupportedOps">
			<member name="MS_OP_NONE" value="0"/>
			<member name="MS_OP_METADATA" value="1"/>
			<member name="MS_OP_RESOLVE" value="2"/>
			<member name="MS_OP_BROWSE" value="4"/>
			<member name="MS_OP_SEARCH" value="8"/>
			<member name="MS_OP_QUERY" value="16"/>
			<member name="MS_OP_STORE" value="32"/>
			<member name="MS_OP_STORE_PARENT" value="64"/>
		</enum>
		<object name="MsContent" parent="GObject" type-name="MsContent" get-type="ms_content_get_type">
			<method name="add" symbol="ms_content_add">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="get" symbol="ms_content_get">
				<return-type type="GValue*"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="get_float" symbol="ms_content_get_float">
				<return-type type="gfloat"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="get_int" symbol="ms_content_get_int">
				<return-type type="gint"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="get_keys" symbol="ms_content_get_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
				</parameters>
			</method>
			<method name="get_overwrite" symbol="ms_content_get_overwrite">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
				</parameters>
			</method>
			<method name="get_string" symbol="ms_content_get_string">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="has_key" symbol="ms_content_has_key">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="key_is_known" symbol="ms_content_key_is_known">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ms_content_new">
				<return-type type="MsContent*"/>
			</constructor>
			<method name="remove" symbol="ms_content_remove">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="set" symbol="ms_content_set">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
					<parameter name="value" type="GValue*"/>
				</parameters>
			</method>
			<method name="set_float" symbol="ms_content_set_float">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
					<parameter name="floatvalue" type="gint"/>
				</parameters>
			</method>
			<method name="set_int" symbol="ms_content_set_int">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
					<parameter name="intvalue" type="gint"/>
				</parameters>
			</method>
			<method name="set_overwrite" symbol="ms_content_set_overwrite">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="overwrite" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_string" symbol="ms_content_set_string">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContent*"/>
					<parameter name="key" type="MsKeyID"/>
					<parameter name="strvalue" type="gchar*"/>
				</parameters>
			</method>
			<property name="overwrite" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="MsContentAudio" parent="MsContentMedia" type-name="MsContentAudio" get-type="ms_content_audio_get_type">
			<constructor name="new" symbol="ms_content_audio_new">
				<return-type type="MsContentMedia*"/>
			</constructor>
		</object>
		<object name="MsContentBox" parent="MsContentMedia" type-name="MsContentBox" get-type="ms_content_box_get_type">
			<method name="get_childcount" symbol="ms_content_box_get_childcount">
				<return-type type="gint"/>
				<parameters>
					<parameter name="content" type="MsContentBox*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="ms_content_box_new">
				<return-type type="MsContentMedia*"/>
			</constructor>
			<method name="set_childcount" symbol="ms_content_box_set_childcount">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContentBox*"/>
					<parameter name="childcount" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="MsContentImage" parent="MsContentMedia" type-name="MsContentImage" get-type="ms_content_image_get_type">
			<constructor name="new" symbol="ms_content_image_new">
				<return-type type="MsContentMedia*"/>
			</constructor>
			<method name="set_size" symbol="ms_content_image_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContentImage*"/>
					<parameter name="width" type="gint"/>
					<parameter name="height" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="MsContentMedia" parent="MsContent" type-name="MsContentMedia" get-type="ms_content_media_get_type">
			<constructor name="new" symbol="ms_content_media_new">
				<return-type type="MsContentMedia*"/>
			</constructor>
			<method name="set_rating" symbol="ms_content_media_set_rating">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContentMedia*"/>
					<parameter name="rating" type="gchar*"/>
					<parameter name="max" type="gchar*"/>
				</parameters>
			</method>
		</object>
		<object name="MsContentVideo" parent="MsContentMedia" type-name="MsContentVideo" get-type="ms_content_video_get_type">
			<constructor name="new" symbol="ms_content_video_new">
				<return-type type="MsContentMedia*"/>
			</constructor>
			<method name="set_size" symbol="ms_content_video_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="content" type="MsContentVideo*"/>
					<parameter name="width" type="gint"/>
					<parameter name="height" type="gint"/>
				</parameters>
			</method>
		</object>
		<object name="MsMediaPlugin" parent="GObject" type-name="MsMediaPlugin" get-type="ms_media_plugin_get_type">
			<method name="get_author" symbol="ms_media_plugin_get_author">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_description" symbol="ms_media_plugin_get_description">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_id" symbol="ms_media_plugin_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_license" symbol="ms_media_plugin_get_license">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="ms_media_plugin_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_site" symbol="ms_media_plugin_get_site">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="get_version" symbol="ms_media_plugin_get_version">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="plugin" type="MsMediaPlugin*"/>
				</parameters>
			</method>
		</object>
		<object name="MsMediaSource" parent="MsMetadataSource" type-name="MsMediaSource" get-type="ms_media_source_get_type">
			<method name="browse" symbol="ms_media_source_browse">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="container" type="MsContentMedia*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="MsMetadataResolutionFlags"/>
					<parameter name="callback" type="MsMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="cancel" symbol="ms_media_source_cancel">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</method>
			<method name="get_auto_split_threshold" symbol="ms_media_source_get_auto_split_threshold">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
				</parameters>
			</method>
			<method name="get_operation_data" symbol="ms_media_source_get_operation_data">
				<return-type type="gpointer"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</method>
			<method name="metadata" symbol="ms_media_source_metadata">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="media" type="MsContentMedia*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="flags" type="MsMetadataResolutionFlags"/>
					<parameter name="callback" type="MsMediaSourceMetadataCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="query" symbol="ms_media_source_query">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="query" type="gchar*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="MsMetadataResolutionFlags"/>
					<parameter name="callback" type="MsMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="search" symbol="ms_media_source_search">
				<return-type type="guint"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="text" type="gchar*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="skip" type="guint"/>
					<parameter name="count" type="guint"/>
					<parameter name="flags" type="MsMetadataResolutionFlags"/>
					<parameter name="callback" type="MsMediaSourceResultCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="set_auto_split_threshold" symbol="ms_media_source_set_auto_split_threshold">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="threshold" type="guint"/>
				</parameters>
			</method>
			<method name="set_operation_data" symbol="ms_media_source_set_operation_data">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
					<parameter name="data" type="gpointer"/>
				</parameters>
			</method>
			<method name="store" symbol="ms_media_source_store">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="parent" type="MsContentBox*"/>
					<parameter name="media" type="MsContentMedia*"/>
					<parameter name="callback" type="MsMediaSourceStoreCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<property name="auto-split-threshold" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<vfunc name="browse">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="bs" type="MsMediaSourceBrowseSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="cancel">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="operation_id" type="guint"/>
				</parameters>
			</vfunc>
			<vfunc name="metadata">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="ms" type="MsMediaSourceMetadataSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="query">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="qs" type="MsMediaSourceQuerySpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="search">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="ss" type="MsMediaSourceSearchSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="store">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMediaSource*"/>
					<parameter name="ss" type="MsMediaSourceStoreSpec*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="MsMetadataSource" parent="MsMediaPlugin" type-name="MsMetadataSource" get-type="ms_metadata_source_get_type">
			<method name="filter_slow" symbol="ms_metadata_source_filter_slow">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="keys" type="GList**"/>
					<parameter name="return_filtered" type="gboolean"/>
				</parameters>
			</method>
			<method name="filter_supported" symbol="ms_metadata_source_filter_supported">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="keys" type="GList**"/>
					<parameter name="return_filtered" type="gboolean"/>
				</parameters>
			</method>
			<method name="get_description" symbol="ms_metadata_source_get_description">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<method name="get_id" symbol="ms_metadata_source_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="ms_metadata_source_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<method name="key_depends" symbol="ms_metadata_source_key_depends">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="key_id" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="resolve" symbol="ms_metadata_source_resolve">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="keys" type="GList*"/>
					<parameter name="media" type="MsContentMedia*"/>
					<parameter name="flags" type="guint"/>
					<parameter name="callback" type="MsMetadataSourceResolveCb"/>
					<parameter name="user_data" type="gpointer"/>
				</parameters>
			</method>
			<method name="slow_keys" symbol="ms_metadata_source_slow_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<method name="supported_keys" symbol="ms_metadata_source_supported_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<method name="supported_operations" symbol="ms_metadata_source_supported_operations">
				<return-type type="MsSupportedOps"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</method>
			<property name="source-desc" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="source-id" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="source-name" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<vfunc name="key_depends">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="key_id" type="MsKeyID"/>
				</parameters>
			</vfunc>
			<vfunc name="resolve">
				<return-type type="void"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
					<parameter name="rs" type="MsMetadataSourceResolveSpec*"/>
				</parameters>
			</vfunc>
			<vfunc name="slow_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="supported_keys">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="supported_operations">
				<return-type type="MsSupportedOps"/>
				<parameters>
					<parameter name="source" type="MsMetadataSource*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="MsPluginRegistry" parent="GObject" type-name="MsPluginRegistry" get-type="ms_plugin_registry_get_type">
			<method name="get_instance" symbol="ms_plugin_registry_get_instance">
				<return-type type="MsPluginRegistry*"/>
			</method>
			<method name="get_sources" symbol="ms_plugin_registry_get_sources">
				<return-type type="MsMediaPlugin**"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
				</parameters>
			</method>
			<method name="load" symbol="ms_plugin_registry_load">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<method name="load_all" symbol="ms_plugin_registry_load_all">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
				</parameters>
			</method>
			<method name="load_directory" symbol="ms_plugin_registry_load_directory">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="path" type="gchar*"/>
				</parameters>
			</method>
			<method name="lookup_metadata_key" symbol="ms_plugin_registry_lookup_metadata_key">
				<return-type type="MsMetadataKey*"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="key_id" type="MsKeyID"/>
				</parameters>
			</method>
			<method name="lookup_source" symbol="ms_plugin_registry_lookup_source">
				<return-type type="MsMediaPlugin*"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="source_id" type="gchar*"/>
				</parameters>
			</method>
			<method name="register_source" symbol="ms_plugin_registry_register_source">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="plugin" type="MsPluginInfo*"/>
					<parameter name="source" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<method name="unload" symbol="ms_plugin_registry_unload">
				<return-type type="void"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="plugin_id" type="gchar*"/>
				</parameters>
			</method>
			<method name="unregister_source" symbol="ms_plugin_registry_unregister_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="registry" type="MsPluginRegistry*"/>
					<parameter name="source" type="MsMediaPlugin*"/>
				</parameters>
			</method>
			<signal name="source-added" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="MsPluginRegistry*"/>
					<parameter name="p0" type="MsMediaPlugin*"/>
				</parameters>
			</signal>
			<signal name="source-removed" when="FIRST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="MsPluginRegistry*"/>
					<parameter name="p0" type="MsMediaPlugin*"/>
				</parameters>
			</signal>
		</object>
		<constant name="MS_KEYID_FORMAT" type="char*" value="u"/>
		<constant name="MS_METADATA_KEY_ALBUM" type="int" value="4"/>
		<constant name="MS_METADATA_KEY_ALBUM_DESC" type="char*" value="Album the media belongs to"/>
		<constant name="MS_METADATA_KEY_ALBUM_NAME" type="char*" value="album"/>
		<constant name="MS_METADATA_KEY_ARTIST" type="int" value="3"/>
		<constant name="MS_METADATA_KEY_ARTIST_DESC" type="char*" value="Main artist"/>
		<constant name="MS_METADATA_KEY_ARTIST_NAME" type="char*" value="artist"/>
		<constant name="MS_METADATA_KEY_AUTHOR" type="int" value="8"/>
		<constant name="MS_METADATA_KEY_AUTHOR_DESC" type="char*" value="Creator of the media"/>
		<constant name="MS_METADATA_KEY_AUTHOR_NAME" type="char*" value="author"/>
		<constant name="MS_METADATA_KEY_CHILDCOUNT" type="int" value="15"/>
		<constant name="MS_METADATA_KEY_CHILDCOUNT_DESC" type="char*" value="Number of items contained in a container"/>
		<constant name="MS_METADATA_KEY_CHILDCOUNT_NAME" type="char*" value="childcount"/>
		<constant name="MS_METADATA_KEY_CHILDCOUNT_UNKNOWN" type="int" value="-1"/>
		<constant name="MS_METADATA_KEY_DATE" type="int" value="14"/>
		<constant name="MS_METADATA_KEY_DATE_DESC" type="char*" value="Publishing or recording date"/>
		<constant name="MS_METADATA_KEY_DATE_NAME" type="char*" value="date"/>
		<constant name="MS_METADATA_KEY_DESCRIPTION" type="int" value="9"/>
		<constant name="MS_METADATA_KEY_DESCRIPTION_DESC" type="char*" value="Description of the media"/>
		<constant name="MS_METADATA_KEY_DESCRIPTION_NAME" type="char*" value="description"/>
		<constant name="MS_METADATA_KEY_DURATION" type="int" value="13"/>
		<constant name="MS_METADATA_KEY_DURATION_DESC" type="char*" value="Media duration"/>
		<constant name="MS_METADATA_KEY_DURATION_NAME" type="char*" value="duration"/>
		<constant name="MS_METADATA_KEY_FRAMERATE" type="int" value="19"/>
		<constant name="MS_METADATA_KEY_FRAMERATE_DESC" type="char*" value="Frames per second"/>
		<constant name="MS_METADATA_KEY_FRAMERATE_NAME" type="char*" value="framerate"/>
		<constant name="MS_METADATA_KEY_GENRE" type="int" value="5"/>
		<constant name="MS_METADATA_KEY_GENRE_DESC" type="char*" value="Genre of the media"/>
		<constant name="MS_METADATA_KEY_GENRE_NAME" type="char*" value="genre"/>
		<constant name="MS_METADATA_KEY_HEIGHT" type="int" value="18"/>
		<constant name="MS_METADATA_KEY_HEIGHT_DESC" type="char*" value="height of video (y-axis resolution)"/>
		<constant name="MS_METADATA_KEY_HEIGHT_NAME" type="char*" value="height"/>
		<constant name="MS_METADATA_KEY_ID" type="int" value="7"/>
		<constant name="MS_METADATA_KEY_ID_DESC" type="char*" value="Identifier of media"/>
		<constant name="MS_METADATA_KEY_ID_NAME" type="char*" value="id"/>
		<constant name="MS_METADATA_KEY_LYRICS" type="int" value="11"/>
		<constant name="MS_METADATA_KEY_LYRICS_DESC" type="char*" value="Song lyrics"/>
		<constant name="MS_METADATA_KEY_LYRICS_NAME" type="char*" value="lyrics"/>
		<constant name="MS_METADATA_KEY_MIME" type="int" value="16"/>
		<constant name="MS_METADATA_KEY_MIME_DESC" type="char*" value="Media mime type"/>
		<constant name="MS_METADATA_KEY_MIME_NAME" type="char*" value="mime-type"/>
		<constant name="MS_METADATA_KEY_RATING" type="int" value="20"/>
		<constant name="MS_METADATA_KEY_RATING_DESC" type="char*" value="Media rating"/>
		<constant name="MS_METADATA_KEY_RATING_NAME" type="char*" value="rating"/>
		<constant name="MS_METADATA_KEY_SITE" type="int" value="12"/>
		<constant name="MS_METADATA_KEY_SITE_DESC" type="char*" value="Site"/>
		<constant name="MS_METADATA_KEY_SITE_NAME" type="char*" value="site"/>
		<constant name="MS_METADATA_KEY_SOURCE" type="int" value="10"/>
		<constant name="MS_METADATA_KEY_SOURCE_DESC" type="char*" value="Source ID providing the content"/>
		<constant name="MS_METADATA_KEY_SOURCE_NAME" type="char*" value="source"/>
		<constant name="MS_METADATA_KEY_THUMBNAIL" type="int" value="6"/>
		<constant name="MS_METADATA_KEY_THUMBNAIL_DESC" type="char*" value="Thumbnail image"/>
		<constant name="MS_METADATA_KEY_THUMBNAIL_NAME" type="char*" value="thumbnail"/>
		<constant name="MS_METADATA_KEY_TITLE" type="int" value="1"/>
		<constant name="MS_METADATA_KEY_TITLE_DESC" type="char*" value="Title of the media"/>
		<constant name="MS_METADATA_KEY_TITLE_NAME" type="char*" value="title"/>
		<constant name="MS_METADATA_KEY_URL" type="int" value="2"/>
		<constant name="MS_METADATA_KEY_URL_DESC" type="char*" value="Media URL"/>
		<constant name="MS_METADATA_KEY_URL_NAME" type="char*" value="url"/>
		<constant name="MS_METADATA_KEY_WIDTH" type="int" value="17"/>
		<constant name="MS_METADATA_KEY_WIDTH_DESC" type="char*" value="Width of video (x-axis resolution)"/>
		<constant name="MS_METADATA_KEY_WIDTH_NAME" type="char*" value="width"/>
		<constant name="MS_PLUGIN_PATH_VAR" type="char*" value="MS_PLUGIN_PATH"/>
	</namespace>
</api>
