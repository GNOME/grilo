/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#ifndef _GRILO_H_
#define _GRILO_H_

#define _GRILO_H_INSIDE_

#include <glib.h>

#include <grl-error.h>
#include <grl-log.h>
#include <grl-registry.h>
#include <grl-plugin.h>
#include <grl-metadata-key.h>
#include <grl-data.h>
#include <grl-media.h>
#include <grl-config.h>
#include <grl-related-keys.h>
#include <grl-source.h>
#include <grl-multiple.h>
#include <grl-util.h>
#include <grl-definitions.h>
#include <grl-operation.h>

#undef _GRILO_H_INSIDE_

G_BEGIN_DECLS

void grl_init (gint *argc, gchar **argv[]);

void grl_deinit (void);

GOptionGroup *grl_init_get_option_group (void);

G_END_DECLS

#endif /* _GRILO_H_ */
