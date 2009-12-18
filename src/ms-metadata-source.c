/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "ms-metadata-source.h"

#include <string.h>

#define MS_METADATA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), MS_TYPE_METADATA_SOURCE, MsMetadataSourcePrivate))

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_DESC,
};

struct _MsMetadataSourcePrivate {
  gchar *id;
  gchar *name;
  gchar *desc;
};

static void ms_metadata_source_finalize (GObject *plugin);
static void ms_metadata_source_get_property (GObject *plugin,
                                             guint prop_id,
                                             GValue *value,
                                             GParamSpec *pspec);
static void ms_metadata_source_set_property (GObject *object, 
                                             guint prop_id,
                                             const GValue *value,
                                             GParamSpec *pspec);

G_DEFINE_ABSTRACT_TYPE (MsMetadataSource, ms_metadata_source, MS_TYPE_MEDIA_PLUGIN);

static void
ms_metadata_source_class_init (MsMetadataSourceClass *metadata_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (metadata_source_class);

  gobject_class->finalize = ms_metadata_source_finalize;
  gobject_class->set_property = ms_metadata_source_set_property;
  gobject_class->get_property = ms_metadata_source_get_property;
  
  /**
   * MsMetadataSource:source-id
   *
   * The identifier of the source.
   */
  g_object_class_install_property (gobject_class,
				   PROP_ID,
				   g_param_spec_string ("source-id",
							"Source identifier",
							"The identifier of the source",
							"",
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));  
  /**
   * MsMetadataSource:source-name
   *
   * The name of the source.
   */
  g_object_class_install_property (gobject_class,
				   PROP_NAME,
				   g_param_spec_string ("source-name",
							"Source name",
							"The name of the source",
							"",
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));  
  /**
   * MsMetadataSource:source-desc
   *
   * A description of the source
   */
  g_object_class_install_property (gobject_class,
				   PROP_DESC,
				   g_param_spec_string ("source-desc",
							"Source description",
							"A description of the source",
							"",
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT |
							G_PARAM_STATIC_STRINGS));  

  g_type_class_add_private (metadata_source_class, sizeof (MsMetadataSourcePrivate));
}

static void
ms_metadata_source_init (MsMetadataSource *source)
{
  source->priv = MS_METADATA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MsMetadataSourcePrivate));
}

static void
ms_metadata_source_finalize (GObject *object)
{
  MsMetadataSource *source;
  
  source = MS_METADATA_SOURCE (object);

  g_free (source->priv->id);
  g_free (source->priv->name);
  g_free (source->priv->desc);
  
  G_OBJECT_CLASS (ms_metadata_source_parent_class)->finalize (object);
}


static void
set_string_property (gchar **property, const GValue *value)
{
  if (*property) {
    g_free (*property);
  }
  *property = g_value_dup_string (value);
}

static void
ms_metadata_source_set_property (GObject *object, 
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
  MsMetadataSource *source;
  
  source = MS_METADATA_SOURCE (object);
  
  switch (prop_id) {
  case PROP_ID:
    set_string_property (&source->priv->id, value);
    break;
  case PROP_NAME:
    set_string_property (&source->priv->name, value);
    break;
  case PROP_DESC:
    set_string_property (&source->priv->desc, value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(source, prop_id, pspec);
    break;
  }
}

static void
ms_metadata_source_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
  MsMetadataSource *source;

  source = MS_METADATA_SOURCE (object);

  switch (prop_id) {
  case PROP_ID:
    g_value_set_string (value, source->priv->id);
    break;
  case PROP_NAME:
    g_value_set_string (value, source->priv->name);
    break;
  case PROP_DESC:
    g_value_set_string (value, source->priv->desc);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(source, prop_id, pspec);
    break;
  }  
}

const GList *
ms_metadata_source_supported_keys (MsMetadataSource *source)
{
  return MS_METADATA_SOURCE_GET_CLASS (source)->supported_keys (source);
}

const GList *
ms_metadata_source_key_depends (MsMetadataSource *source, MsKeyID key_id)
{
  return MS_METADATA_SOURCE_GET_CLASS (source)->key_depends (source, key_id);
}

void
ms_metadata_source_get (MsMetadataSource *source,
		     const gchar *object_id,
		     const GList *keys,
		     MsMetadataSourceResultCb callback,
		     gpointer user_data)
{
  MS_METADATA_SOURCE_GET_CLASS (source)->metadata (source,
                                                   object_id,
                                                   keys,
                                                   callback, user_data);
}

void
ms_metadata_source_resolve (MsMetadataSource *source, 
                            const GList *keys, 
                            MsContent *media,
                            MsMetadataSourceResolveCb callback,
                            gpointer user_data)
{
  MS_METADATA_SOURCE_GET_CLASS (source)->resolve (source,
                                                  keys,
                                                  media,
                                                  callback,
                                                  user_data);
}

GList *
ms_metadata_source_filter_supported (MsMetadataSource *source, GList **keys)
{
  const GList *supported_keys;
  GList *iter_supported;
  GList *iter_keys;
  MsKeyID key;
  GList *filtered_keys = NULL;
  gboolean got_match;
  GList *iter_keys_prev;
  MsKeyID supported_key;

  supported_keys = ms_metadata_source_supported_keys (source);

  iter_keys = *keys;
  while (iter_keys) {
    got_match = FALSE;
    iter_supported = (GList *) supported_keys;

    key = GPOINTER_TO_INT (iter_keys->data);
    while (!got_match && iter_supported) {
      supported_key = GPOINTER_TO_INT (iter_supported->data);
      if (key == supported_key) {
	got_match = TRUE;
      }
      iter_supported = g_list_next (iter_supported);
    }

    iter_keys_prev = iter_keys;
    iter_keys = g_list_next (iter_keys);
    
    if (got_match) {
      filtered_keys = g_list_prepend (filtered_keys, GINT_TO_POINTER (key));
      *keys = g_list_delete_link (*keys, iter_keys_prev);
      got_match = FALSE;
    }
  }

  return filtered_keys;
}
