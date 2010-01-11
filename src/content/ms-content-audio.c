/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Juan A. Suarez Romero <jasuarez@igalia.com>
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

/*
 * A multimedia content for audio.
 *
 * This high level class represents an audio multimedia item. It has methods to
 * set and get properties like album, and so on.
 *
 */

#include "ms-content-audio.h"


static void ms_content_audio_dispose (GObject *object);
static void ms_content_audio_finalize (GObject *object);

G_DEFINE_TYPE (MsContentAudio, ms_content_audio, MS_TYPE_CONTENT_MEDIA);

static void
ms_content_audio_class_init (MsContentAudioClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;

    gobject_class->dispose = ms_content_audio_dispose;
    gobject_class->finalize = ms_content_audio_finalize;
}

static void
ms_content_audio_init (MsContentAudio *self)
{
}

static void
ms_content_audio_dispose (GObject *object)
{
    G_OBJECT_CLASS (ms_content_audio_parent_class)->dispose (object);
}

static void
ms_content_audio_finalize (GObject *object)
{
    g_signal_handlers_destroy (object);
    G_OBJECT_CLASS (ms_content_audio_parent_class)->finalize (object);
}

/**
 * ms_content_audio_new:
 *
 * Creates a new content audio object.
 *
 * Returns: a newly-allocated content audio.
 **/
MsContentMedia *
ms_content_audio_new (void)
{
  return MS_CONTENT_MEDIA (g_object_new (MS_TYPE_CONTENT_AUDIO,
                                         NULL));
}
