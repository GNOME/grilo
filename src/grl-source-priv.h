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

#ifndef _GRL_SOURCE_PRIV_H_
#define _GRL_SOURCE_PRIV_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "grl-source.h"

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

GList *grl_source_filter_supported (GrlSource *source,
                                    GList **keys,
                                    gboolean return_filtered);

GList *grl_source_filter_slow (GrlSource *source,
                               GList **keys,
                               gboolean return_filtered);

GList *grl_source_filter_writable (GrlSource *source,
                                   GList **keys,
                                   gboolean return_filtered);

void grl_source_set_operation_finished (GrlSource *source,
                                        guint operation_id);

gboolean grl_source_operation_is_finished (GrlSource *source,
                                           guint operation_id);

void grl_source_set_operation_completed (GrlSource *source,
                                         guint operation_id);

gboolean grl_source_operation_is_completed (GrlSource *source,
                                            guint operation_id);

void grl_source_set_operation_cancelled (GrlSource *source,
                                         guint operation_id);

gboolean grl_source_operation_is_cancelled (GrlSource *source,
                                            guint operation_id);

void grl_source_set_operation_ongoing (GrlSource *source,
                                       guint operation_id);

gboolean grl_source_operation_is_ongoing (GrlSource *source,
                                          guint operation_id);

G_END_DECLS

#endif /* _GRL_SOURCE_PRIV_H_ */
