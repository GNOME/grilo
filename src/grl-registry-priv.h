/*
 * Copyright (C) 2011 Igalia S.L.
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

#ifndef _GRL_REGISTRY_PRIV_H_
#define _GRL_REGISTRY_PRIV_H_

#include <grl-registry.h>

void
grl_registry_restrict_plugins (GrlRegistry *registry,
                               gchar **plugins);

GrlKeyID grl_registry_register_metadata_key_system (GrlRegistry *registry,
                                                    GParamSpec *param_spec,
                                                    GrlKeyID key,
                                                    GrlKeyID bind_key,
                                                    GError **error);

void grl_registry_shutdown (GrlRegistry *registry);

GrlKeyID grl_registry_register_metadata_key_for_type (GrlRegistry *registry,
                                                      const gchar *key_name,
                                                      GType type,
                                                      GrlKeyID bind_key);

GrlKeyID grl_registry_register_or_lookup_metadata_key (GrlRegistry *registry,
                                                       const gchar *key_name,
                                                       const GValue *value,
                                                       GrlKeyID bind_key);

gboolean grl_registry_metadata_key_get_limits(GrlRegistry *registry,
                                              GrlKeyID key,
                                              GValue *min,
                                              GValue *max);

gboolean grl_registry_metadata_key_clamp(GrlRegistry *registry,
                                         GrlKeyID key,
                                         GValue *min,
                                         GValue *value,
                                         GValue *max);

gboolean grl_registry_metadata_key_is_max_valid(GrlRegistry *registry,
                                                GrlKeyID key,
                                                GValue *min,
                                                GValue *max);

#endif /* _GRL_REGISTRY_PRIV_H_ */
