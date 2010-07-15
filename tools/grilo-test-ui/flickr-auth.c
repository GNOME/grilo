/*
 * Copyright (C) 2010 Igalia S.L.
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

#include "flickr-auth.h"

#include <gio/gio.h>
#include <string.h>
#include <libxml/xpath.h>

#define FLICKR_ENDPOINT  "http://api.flickr.com/services/rest/?"
#define FLICKR_AUTHPOINT "http://flickr.com/services/auth/?"

#define FLICKR_AUTH_GETFROB_METHOD  "flickr.auth.getFrob"
#define FLICKR_AUTH_GETTOKEN_METHOD "flickr.auth.getToken"

#define FLICKR_AUTH_GETFROB                     \
  FLICKR_ENDPOINT                               \
  "api_key=%s"                                  \
  "&api_sig=%s"                                 \
  "&method=" FLICKR_AUTH_GETFROB_METHOD

#define FLICKR_AUTH_GETTOKEN                    \
  FLICKR_ENDPOINT                               \
  "api_key=%s"                                  \
  "&api_sig=%s"                                 \
  "&method=" FLICKR_AUTH_GETTOKEN_METHOD        \
  "&frob=%s"

#define FLICKR_AUTH_LOGINLINK                   \
  FLICKR_AUTHPOINT                              \
  "api_key=%s"                                  \
  "&api_sig=%s"                                 \
  "&frob=%s"                                    \
  "&perms=%s"

static gchar *
get_xpath_element (const gchar *content,
                   const gchar *xpath_element)
{
  gchar *element = NULL;
  xmlDocPtr xmldoc = NULL;
  xmlXPathContextPtr xpath_ctx = NULL;
  xmlXPathObjectPtr xpath_res = NULL;

  xmldoc = xmlReadMemory (content, xmlStrlen ((xmlChar *) content), NULL, NULL,
                          XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);
  if (xmldoc) {
    xpath_ctx = xmlXPathNewContext (xmldoc);
    if (xpath_ctx) {
      xpath_res = xmlXPathEvalExpression ((xmlChar *) xpath_element, xpath_ctx);
      if (xpath_res && xpath_res->nodesetval->nodeTab) {
        element =
          (gchar *) xmlNodeListGetString (xmldoc,
                                          xpath_res->nodesetval->nodeTab[0]->xmlChildrenNode,
                                          1);
      }
    }
  }

  /* Free data */
  if (xmldoc) {
    xmlFreeDoc (xmldoc);
  }

  if (xpath_ctx) {
    xmlXPathFreeContext (xpath_ctx);
  }

  if (xpath_res) {
    xmlXPathFreeObject (xpath_res);
  }

  return element;
}

static gchar *
get_api_sig (const gchar *secret, ...)
{
  GHashTable *hash;
  GList *key_iter;
  GList *keys;
  GString *to_sign;
  gchar *api_sig;
  gchar *key;
  gchar *value;
  gint text_size = strlen (secret);
  va_list va_params;

  hash = g_hash_table_new (g_str_hash, g_str_equal);

  va_start (va_params, secret);
  while ((key = va_arg (va_params, gchar *))) {
    text_size += strlen (key);
    value = va_arg (va_params, gchar *);
    text_size += strlen (value);
    g_hash_table_insert (hash, key, value);
  }
  va_end (va_params);

  to_sign = g_string_sized_new (text_size);
  g_string_append (to_sign, secret);

  keys = g_hash_table_get_keys (hash);
  keys = g_list_sort (keys, (GCompareFunc) g_strcmp0);
  for (key_iter = keys; key_iter; key_iter = g_list_next (key_iter)) {
    g_string_append (to_sign, key_iter->data);
    g_string_append (to_sign, g_hash_table_lookup (hash, key_iter->data));
  }

  api_sig = g_compute_checksum_for_string (G_CHECKSUM_MD5, to_sign->str, -1);
  g_hash_table_unref (hash);
  g_list_free (keys);
  g_string_free (to_sign, TRUE);

  return api_sig;
}

gchar *
flickr_get_frob (const gchar *key,
                 const gchar *secret)
{
  gchar *api_sig;
  gchar *url;
  GVfs *vfs;
  GFile *uri;
  gchar *contents;
  GError *error = NULL;
  gchar *frob = NULL;

  g_return_val_if_fail (key, NULL);
  g_return_val_if_fail (secret, NULL);

  api_sig = get_api_sig (secret,
                         "api_key", key,
                         "method", FLICKR_AUTH_GETFROB_METHOD,
                         NULL);

  /* Build url */
  url = g_strdup_printf (FLICKR_AUTH_GETFROB, key, api_sig);
  g_free (api_sig);

  /* Load content */
  vfs = g_vfs_get_default ();
  uri = g_vfs_get_file_for_uri (vfs, url);
  g_free (url);
  if (!g_file_load_contents (uri, NULL, &contents, NULL, NULL, &error)) {
    g_warning ("Unable to get Flickr's frob: %s", error->message);
    return NULL;
  }

  /* Get frob */
  frob = get_xpath_element (contents, "/rsp/frob");
  g_free (contents);
  if (!frob) {
    g_warning ("Can not get Flickr's frob");
  }

  return frob;
}

gchar *
flickr_get_token (const gchar *key,
                  const gchar *secret,
                  const gchar *frob)
{
  GError *error = NULL;
  GFile *uri;
  GVfs *vfs;
  gchar *api_sig;
  gchar *contents;
  gchar *token;
  gchar *url;

  g_return_val_if_fail (key, NULL);
  g_return_val_if_fail (secret, NULL);
  g_return_val_if_fail (frob, NULL);

  api_sig = get_api_sig (secret,
                         "method", FLICKR_AUTH_GETTOKEN_METHOD,
                         "api_key", key,
                         "frob", frob,
                         NULL);

  /* Build url */
  url = g_strdup_printf (FLICKR_AUTH_GETTOKEN,
                         key,
                         api_sig,
                         frob);
  g_free (api_sig);

  /* Load content */
  vfs = g_vfs_get_default ();
  uri = g_vfs_get_file_for_uri (vfs, url);
  g_free (url);
  if (!g_file_load_contents (uri, NULL, &contents, NULL, NULL, &error)) {
    g_warning ("Unable to get Flickr's token: %s", error->message);
    return NULL;
  }

  /* Get token */
  token = get_xpath_element (contents, "/rsp/auth/token");
  g_free (contents);
  if (!token) {
    g_warning ("Can not get Flickr's token");
  }

  return token;
}

gchar *
flickr_get_login_link (const gchar *key,
                       const gchar *secret,
                       const gchar *frob,
                       const gchar *perm)
{
  gchar *api_sig;
  gchar *url;

  g_return_val_if_fail (key, NULL);
  g_return_val_if_fail (secret, NULL);
  g_return_val_if_fail (frob, NULL);
  g_return_val_if_fail (perm, NULL);

  api_sig = get_api_sig (secret,
                         "api_key", key,
                         "frob", frob,
                         "perms", perm,
                         NULL);

  url = g_strdup_printf (FLICKR_AUTH_LOGINLINK,
                         key,
                         api_sig,
                         frob,
                         perm);
  g_free (api_sig);

  return url;
}
