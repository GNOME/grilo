/*
 * Copyright (C) 2010, 2011 Igalia S.L.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
 *          Jens Georg <jensg@openismus.com>
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

#ifndef _GRL_NET_WC_H_
#define _GRL_NET_WC_H_

#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * GrlNetWcError:
 * @GRL_NET_WC_ERROR_UNAVAILABLE: Deprecated. For generic errors, you
 * should check for G_IO_ERROR_FAILED from G_IO_ERROR domain.
 * @GRL_NET_WC_ERROR_PROTOCOL_ERROR: Invalid URI or header
 * @GRL_NET_WC_ERROR_AUTHENTICATION_REQUIRED: Required authentication
 * @GRL_NET_WC_ERROR_NOT_FOUND: Request resource not found
 * @GRL_NET_WC_ERROR_CONFLICT: The entry has been modified since is was
 * downloaded
 * @GRL_NET_WC_ERROR_FORBIDDEN: TBD
 * @GRL_NET_WC_ERROR_NETWORK_ERROR: Cannot connect to the server
 * @GRL_NET_WC_ERROR_PROXY_ERROR: Deprecated. You should check for
 * G_IO_ERROR_PROXY_FAILED from G_IO_ERROR domain.
 * @GRL_NET_WC_ERROR_CANCELLED: Deprecated. You should check for
 * G_IO_ERROR_CANCELLED from G_IO_ERROR domain.
 *
 * These constants identify all the available errors managed by
 * the web client.
 */
typedef enum {
	GRL_NET_WC_ERROR_UNAVAILABLE = 1,
	GRL_NET_WC_ERROR_PROTOCOL_ERROR,
	GRL_NET_WC_ERROR_AUTHENTICATION_REQUIRED,
	GRL_NET_WC_ERROR_NOT_FOUND,
	GRL_NET_WC_ERROR_CONFLICT,
	GRL_NET_WC_ERROR_FORBIDDEN,
	GRL_NET_WC_ERROR_NETWORK_ERROR,
	GRL_NET_WC_ERROR_PROXY_ERROR,
	GRL_NET_WC_ERROR_CANCELLED
} GrlNetWcError;

#define GRL_TYPE_NET_WC				\
  (grl_net_wc_get_type ())

#define GRL_NET_WC(obj)						\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRL_TYPE_NET_WC, GrlNetWc))

#define GRL_IS_NET_WC(obj)					\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GRL_TYPE_NET_WC))

#define GRL_NET_WC_CLASS(klass)					\
  (G_TYPE_CHECK_CLASS_CAST((klass), GRL_TYPE_NET_WC, GrlNetWcClass))

#define GRL_IS_NET_WC_CLASS(klass)				\
  (G_TYPE_CHECK_CLASS_TYPE((klass), GRL_TYPE_NET_WC))

#define GRL_NET_WC_GET_CLASS(obj)					\
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GRL_TYPE_NET_WC, GrlNetWcClass))

#define GRL_NET_WC_ERROR grl_net_wc_error_quark ()

typedef struct _GrlNetWc        GrlNetWc;
typedef struct _GrlNetWcClass   GrlNetWcClass;
typedef struct _GrlNetWcPrivate GrlNetWcPrivate;

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlNetWc, g_object_unref)

/**
 * GrlNetWcClass:
 * @parent_class: the parent class structure
 *
 * Grilo web client helper class.
 *
 * It's a simple and thin web client to be used by the sources to download
 * content from Internet.
 */
struct _GrlNetWcClass
{
  GObjectClass parent_class;
};

GType grl_net_wc_get_type (void) G_GNUC_CONST;

GQuark grl_net_wc_error_quark (void) G_GNUC_CONST;

GrlNetWc *grl_net_wc_new (void);

void grl_net_wc_request_async (GrlNetWc *self,
			       const char *uri,
			       GCancellable *cancellable,
			       GAsyncReadyCallback callback,
			       gpointer user_data);

void grl_net_wc_request_with_headers_hash_async (GrlNetWc *self,
                                                 const char *uri,
                                                 GHashTable *headers,
                                                 GCancellable *cancellable,
                                                 GAsyncReadyCallback callback,
                                                 gpointer user_data);

void grl_net_wc_request_with_headers_async (GrlNetWc *self,
                                            const char *uri,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer user_data,
                                            ...) G_GNUC_NULL_TERMINATED;

gboolean grl_net_wc_request_finish (GrlNetWc *self,
				    GAsyncResult *result,
				    gchar **content,
				    gsize *length,
				    GError **error);

void grl_net_wc_set_log_level (GrlNetWc *self,
			       guint log_level);

void grl_net_wc_set_throttling (GrlNetWc *self,
				guint throttling);

void grl_net_wc_set_cache (GrlNetWc *self,
                           gboolean use_cache);

void grl_net_wc_set_cache_size (GrlNetWc *self,
                                guint cache_size);

void grl_net_wc_flush_delayed_requests (GrlNetWc *self);

G_END_DECLS

#endif /* _GRL_NET_WC_H_ */
