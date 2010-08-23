/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * Contact: Iago Toral Quiroga <itoral@igalia.com>
 *
 * Authors: Víctor M. Jáquez L. <vjaquez@igalia.com>
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

typedef enum {
	GRL_NET_WC_ERROR_UNAVAILABLE = 1,
	GRL_NET_WC_ERROR_PROTOCOL_ERROR,
	GRL_NET_WC_ERROR_AUTHENTICATION_REQUIRED,
	GRL_NET_WC_ERROR_NOT_FOUND,
	GRL_NET_WC_ERROR_CONFLICT,
	GRL_NET_WC_ERROR_FORBIDDEN,
	GRL_NET_WC_ERROR_NETWORK_ERROR,
	GRL_NET_WC_ERROR_PROXY_ERROR,
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
typedef struct _GrlNetWcPrivate GrlNetWcPrivate;

struct _GrlNetWc {

  GObject parent;

  /*< private >*/
  GrlNetWcPrivate *priv;

};

typedef struct _GrlNetWcClass GrlNetWcClass;

struct _GrlNetWcClass {

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

gboolean grl_net_wc_request_finish (GrlNetWc *self,
				    GAsyncResult *result,
				    gchar **content,
				    gsize *length,
				    GError **error);

void grl_net_wc_set_log_level (GrlNetWc *self,
			       guint log_level);

void grl_net_wc_set_throttling (GrlNetWc *self,
				guint throttling);

void grl_net_wc_flush_delayed_requests (GrlNetWc *self);

G_END_DECLS

#endif /* _GRL_NET_WC_H_ */
