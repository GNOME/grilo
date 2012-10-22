/*
 * Copyright (C) 2012 Openismus GmbH
 *
 * Authors: Jens Georg <jensg@openismus.com>
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

#ifndef _GRL_NET_MOCK_PRIVATE_H_
#define _GRL_NET_MOCK_PRIVATE_H_

#include "grl-net-wc.h"

#define GRL_NET_MOCKED_VAR "GRL_NET_MOCKED"
#define GRL_NET_MOCK_VERSION 1

G_GNUC_INTERNAL
gboolean is_mocked (void);

G_GNUC_INTERNAL
void get_url_mocked (GrlNetWc *self,
                     const char *url,
                     GHashTable *headers,
                     GAsyncResult *result,
                     GCancellable *cancellable);

G_GNUC_INTERNAL
void get_content_mocked (GrlNetWc *self,
                         void *op,
                         gchar **content,
                         gsize *length);

G_GNUC_INTERNAL
void init_mock_requester (GrlNetWc *self);

G_GNUC_INTERNAL
void finalize_mock_requester (GrlNetWc *self);

G_GNUC_INTERNAL
void free_mock_op_res (void *op);

#endif /* _GRL_NET_MOCK_PRIVATE_H_ */
