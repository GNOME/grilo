/*
 * Copyright (C) 2010-2012 Igalia S.L.
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
 * Protected API for GrlPlugin class
 */

#ifndef _GRL_PLUGIN_PRIV_H_
#define _GRL_PLUGIN_PRIV_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-plugin.h"
#include "grl-registry.h"

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void grl_plugin_set_desc (GrlPlugin *plugin,
                          GrlPluginDescriptor *desc);

void grl_plugin_set_load_func (GrlPlugin *plugin,
                               GrlPluginInitFunc load_function);

void grl_plugin_set_unload_func (GrlPlugin *plugin,
                                 GrlPluginDeinitFunc unload_function);

void grl_plugin_set_register_keys_func (GrlPlugin *plugin,
                                        GrlPluginRegisterKeysFunc register_keys_function);

gboolean grl_plugin_load (GrlPlugin *plugin, GList *configurations);

void grl_plugin_unload (GrlPlugin *plugin);

void grl_plugin_register_keys (GrlPlugin *plugin);

void grl_plugin_set_id (GrlPlugin *plugin,
                        const gchar *id);

void grl_plugin_set_filename (GrlPlugin *plugin,
                              const gchar *filename);

void grl_plugin_set_module (GrlPlugin *plugin,
                            GModule *module);

void grl_plugin_set_module_name (GrlPlugin *plugin,
                                 const gchar *module_name);

G_END_DECLS

#endif /* _GRL_PLUGIN_PRIV_H_ */
