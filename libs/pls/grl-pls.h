/*
 * Copyright (C) 2013 Collabora Ltd.
 *
 * Authors: Mateu Batle Sastre <mateu.batle@collabora.com>
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

#ifndef _GRL_PLS_H_
#define _GRL_PLS_H_

#include <grilo.h>
#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * GrlPlsFilterFunc:
 * @source: the #GrlSource the browse call came from
 * @media: (transfer full): a #GrlMedia to operate on
 * @user_data: user data passed to the browse call
 *
 * Callback type to filter, or modify #GrlMedia created
 * when parsing a playlist using one of grl_pls_browse(),
 * grl_pls_browse_sync() or grl_pls_browse_by_spec().
 *
 * The callback is responsible for unreffing @media when returning %NULL or
 * another #GrlMedia.
 *
 * Returns: (transfer full): %NULL to not add this entry to the results,
 *   or a new #GrlMedia populated with metadata of your choice.
 */
typedef GrlMedia * (*GrlPlsFilterFunc) (GrlSource *source,
                                        GrlMedia  *media,
                                        gpointer   user_data);

gboolean grl_pls_media_is_playlist (GrlMedia *media);

void grl_pls_browse_by_spec (GrlSource *source,
                             GrlPlsFilterFunc filter_func,
                             GrlSourceBrowseSpec *bs);

guint grl_pls_browse (GrlSource *source,
                      GrlMedia *playlist,
                      const GList *keys,
                      GrlOperationOptions *options,
                      GrlPlsFilterFunc filter_func,
                      GrlSourceResultCb callback,
                      gpointer user_data);

GList *grl_pls_browse_sync (GrlSource *source,
                            GrlMedia *playlist,
                            const GList *keys,
                            GrlOperationOptions *options,
                            GrlPlsFilterFunc filter_func,
                            GError **error);

GrlMedia * grl_pls_file_to_media (GrlMedia            *content,
                                  GFile               *file,
                                  GFileInfo           *info,
                                  gboolean             handle_pls,
                                  GrlOperationOptions *options);

const char * grl_pls_get_file_attributes (void);

G_END_DECLS

#endif /* _GRL_PLS_H_ */
