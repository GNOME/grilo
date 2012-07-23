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

GrlKeyID grl_registry_register_metadata_key_full (GrlRegistry *registry,
                                                  GParamSpec *param_spec,
                                                  GrlKeyID key,
                                                  GError **error);

#endif /* _GRL_REGISTRY_PRIV_H_ */
