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

#ifndef _GRL_NET_MOCK_H_
#define _GRL_NET_MOCK_H_

#include "grl-net-wc.h"

#define GRL_ENV_NET_MOCKED "GRL_NET_MOCKED"

#define GRL_NET_IS_MOCKED (g_getenv (GRL_ENV_NET_MOCKED))

void get_url_mocked (GrlNetWc *self,
                     const char *url,
                     GHashTable *headers,
                     GAsyncResult *result,
                     GCancellable *cancellable) G_GNUC_INTERNAL;

void get_content_mocked (GrlNetWc *self,
                         void *op,
                         gchar **content,
                         gsize *length) G_GNUC_INTERNAL;

void init_mock_requester (GrlNetWc *self) G_GNUC_INTERNAL;

void finalize_mock_requester (GrlNetWc *self) G_GNUC_INTERNAL;

void free_mock_op_res (void *op) G_GNUC_INTERNAL;

#endif /* _GRL_NET_MOCK_H_ */
