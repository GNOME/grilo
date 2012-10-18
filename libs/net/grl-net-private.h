/*
 * Copyright (C) 2012 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Juan A. Suarez Romero <jasuarez@igalia.com>
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

#ifndef _GRL_NET_PRIVATE_H_
#define _GRL_NET_PRIVATE_H_

#include <libsoup/soup.h>
#include <grilo.h>

#include "grl-net-wc.h"

G_BEGIN_DECLS

#define GRL_LOG_DOMAIN_DEFAULT wc_log_domain
GRL_LOG_DOMAIN_EXTERN(wc_log_domain);

#define GRL_NET_CAPTURE_DIR_VAR "GRL_NET_CAPTURE_DIR"


struct _GrlNetWcPrivate {
  SoupSession *session;
  SoupLoggerLogLevel log_level;
  guint throttling;             /* throttling in secs */
  GTimeVal last_request;        /* last request time  */
  GQueue *pending;              /* closure queue for delayed requests */
  guint cache_size;             /* cache size in Mb */
  void *requester;
  gchar *previous_data;
};

void parse_error (guint status,
		  const gchar *reason,
		  const gchar *response,
		  GSimpleAsyncResult *result);

void get_url_now (GrlNetWc *self,
                  const char *url,
                  GHashTable *headers,
                  GAsyncResult *result,
                  GCancellable *cancellable);

void get_content (GrlNetWc *self,
                  void *op,
                  gchar **content,
                  gsize *length);

void init_requester (GrlNetWc *self);

void finalize_requester (GrlNetWc *self);

void cache_down (GrlNetWc *self);

void cache_up (GrlNetWc *self);

gboolean cache_is_available (GrlNetWc *self);

void cache_set_size (GrlNetWc *self, guint size);

guint cache_get_size (GrlNetWc *self);

void free_op_res (void *op);

G_GNUC_INTERNAL
void init_dump_directory (void);

G_GNUC_INTERNAL
void dump_data (SoupURI *uri, const gchar *data, gsize length);

G_END_DECLS

#endif /* _GRL_NET_PRIVATE_H_ */
