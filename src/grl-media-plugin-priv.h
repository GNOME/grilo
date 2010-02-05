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

/*
 * Protected API for GrlMediaPlugin class
 */

#ifndef _GRL_MEDIA_PLUGIN_PRIV_H_
#define _GRL_MEDIA_PLUGIN_PRIV_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-media-plugin.h"
#include "grl-plugin-registry.h"

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void grl_media_plugin_set_plugin_info (GrlMediaPlugin *plugin,
                                       const GrlPluginInfo *info);

G_END_DECLS

#endif /* _GRL_MEDIA_PLUGIN_PRIV_H_ */
