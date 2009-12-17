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

#include "metadata-source.h"

#include <string.h>

#define METADATA_SOURCE_GET_PRIVATE(object)				\
  (G_TYPE_INSTANCE_GET_PRIVATE((object), METADATA_SOURCE_TYPE, MetadataSourcePrivate))

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  PROP_DESC,
};

struct _MetadataSourcePrivate {
  gchar *id;
  gchar *name;
  gchar *desc;
};

static void metadata_source_finalize (GObject *plugin);
static void metadata_source_get_property (GObject *plugin,
					  guint prop_id,
					  GValue *value,
					  GParamSpec *pspec);
static void metadata_source_set_property (GObject *object, 
					  guint prop_id,
					  const GValue *value,
					  GParamSpec *pspec);

G_DEFINE_ABSTRACT_TYPE (MetadataSource, metadata_source, MEDIA_PLUGIN_TYPE);

static void
metadata_source_class_init (MetadataSourceClass *metadata_source_class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (metadata_source_class);

  gobject_class->finalize = metadata_source_finalize;
  gobject_class->set_property = metadata_source_set_property;
  gobject_class->get_property = metadata_source_get_property;
  
  /**
   * MetadataSource:source-id
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
   * MetadataSource:source-name
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
   * MetadataSource:source-desc
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

  g_type_class_add_private (metadata_source_class, sizeof (MetadataSourcePrivate));
}

static void
metadata_source_init (MetadataSource *source)
{
  source->priv = METADATA_SOURCE_GET_PRIVATE (source);
  memset (source->priv, 0, sizeof (MetadataSourcePrivate));
}

static void
metadata_source_finalize (GObject *object)
{
  MetadataSource *source;
  
  source = METADATA_SOURCE (object);

  g_free (source->priv->id);
  g_free (source->priv->name);
  g_free (source->priv->desc);
  
  G_OBJECT_CLASS (metadata_source_parent_class)->finalize (object);
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
metadata_source_set_property (GObject *object, 
			      guint prop_id,
			      const GValue *value,
			      GParamSpec *pspec)
{
  MetadataSource *source;
  
  source = METADATA_SOURCE (object);
  
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
metadata_source_get_property (GObject *object,
			      guint prop_id,
			      GValue *value,
			      GParamSpec *pspec)
{
  MetadataSource *source;

  source = METADATA_SOURCE (object);

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

const KeyID *
metadata_source_supported_keys (MetadataSource *source)
{
  return METADATA_SOURCE_GET_CLASS (source)->supported_keys (source);
}

KeyID *
metadata_source_key_depends (MetadataSource *source, KeyID key_id)
{
  return METADATA_SOURCE_GET_CLASS (source)->key_depends (source, key_id);
}

void
metadata_source_get (MetadataSource *source,
		     const gchar *object_id,
		     const KeyID *keys,
		     MetadataSourceResultCb callback,
		     gpointer user_data)
{
  METADATA_SOURCE_GET_CLASS (source)->metadata (source,
						object_id,
						keys,
						callback, user_data);
}

void
metadata_source_resolve (MetadataSource *source, 
			 KeyID *keys, 
			 Content *media)
{
  /* TODO: missing the callbacks! */
  METADATA_SOURCE_GET_CLASS (source)->resolve (source, keys, media);
}

GList *
metadata_source_filter_supported (MetadataSource *source, GList **keys)
{
  const KeyID *supported_keys;
  KeyID *iter_supported;
  GList *iter_keys;
  KeyID key;
  GList *filtered_keys = NULL;
  gboolean got_match;

  supported_keys = metadata_source_supported_keys (source);

  iter_keys = *keys;
  while (iter_keys) {
    got_match = FALSE;
    iter_supported = (KeyID *) supported_keys;
    key = GPOINTER_TO_INT (iter_keys->data);

    while (!got_match && *iter_supported) {
      if (key == *iter_supported) {
	got_match = TRUE;
      }
      iter_supported++;
    }

    iter_keys = g_list_next (iter_keys);
    
    if (got_match) {
      filtered_keys = g_list_prepend (filtered_keys, GINT_TO_POINTER (key));
      *keys = g_list_delete_link (*keys, g_list_previous (iter_keys));
      got_match = FALSE;
    }
  }

  return filtered_keys;
}
