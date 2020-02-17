#include "flickr-oauth.h"

#include <glib.h>
#include <stdlib.h>

#include <string.h>
#include <oauth.h>

/* ---------- private functions declarations ---------- */

static gchar *
parse_request_token (const gchar *string, gchar **secret);

static gchar *
parse_access_token (const gchar *string, gchar **secret);

static gchar *
parse_token (const gint parts_no, const gchar *string, gchar **secret);

static gchar *
get_param_value (const gchar *string);

static gchar *
get_timestamp (void);

static void
free_params (gchar **params, gint params_no);

/* ---------- public API  ---------- */

gchar *
flickroauth_get_signature (const gchar *consumer_secret,
                           const gchar *token_secret,
                           const gchar *url,
                           gchar **params,
                           gint params_no)
{
  gchar *params_string;
  gchar *base_string;
  gchar *encryption_key;
  gchar *signature;

  qsort (params, params_no, sizeof (gchar *), oauth_cmpstringp);

  params_string = oauth_serialize_url (params_no, 0, params);

  base_string = oauth_catenc (3, FLICKR_OAUTH_HTTP_METHOD,
                                url, params_string);

  g_free (params_string);

  if (token_secret == NULL)
    encryption_key = g_strdup_printf ("%s&", consumer_secret);
  else
    encryption_key = g_strdup_printf ("%s&%s", consumer_secret, token_secret);

  signature = oauth_sign_hmac_sha1 (base_string, encryption_key);

  g_free (encryption_key);
  g_free (base_string);

  return signature;
}

gchar *
flickroauth_get_request_token (const gchar *consumer_key,
                               const gchar *consumer_secret,
                               gchar **secret)
{
  gchar *signature;
  gchar *url;
  gchar *timestamp;
  gchar *nonce;
  gchar *params[7];
  gchar *params_string; /* one string later created from params[] */
  gchar *http_reply;
  gchar *request_token = NULL;

  timestamp = get_timestamp ();
  nonce = oauth_gen_nonce ();

  params[0] = g_strdup_printf ("oauth_callback=%s", FLICKR_OAUTH_CALLBACK);
  params[1] = g_strdup_printf ("oauth_consumer_key=%s", consumer_key);
  params[2] = g_strdup_printf ("oauth_nonce=%s", nonce);
  params[3] = g_strdup_printf ("oauth_signature_method=%s",
                              FLICKR_OAUTH_SIGNATURE_METHOD);
  params[4] = g_strdup_printf ("oauth_timestamp=%s", timestamp);
  params[5] = g_strdup_printf ("oauth_version=%s", FLICKR_OAUTH_VERSION);


  g_free (timestamp);
  g_free (nonce);

  signature = flickroauth_get_signature (consumer_secret, NULL,
                                         FLICKR_OAUTH_REQUESTTOKEN_URL,
                                         params, 6);

  params[6] = g_strdup_printf ("oauth_signature=%s", signature);
  g_free (signature);

  params_string = oauth_serialize_url (7, 0, params);

  free_params (params, 7);

  url = g_strdup_printf ("%s?%s", FLICKR_OAUTH_REQUESTTOKEN_URL, params_string);
  g_free (params_string);

  http_reply = oauth_http_get2 (url, NULL, NULL);
  g_free (url);

  if (http_reply) {
    request_token = parse_request_token (http_reply, secret);
    g_free (http_reply);
  }

  return request_token;
}

gchar *
flickroauth_get_access_token (const gchar *consumer_key,
                              const gchar *consumer_secret,
                              const gchar *oauth_token,
                              const gchar *oauth_token_secret,
                              const gchar *verifier,
                              gchar **secret)
{
  gchar *signature;
  gchar *url;
  gchar *timestamp;
  gchar *nonce;
  gchar *params[8];
  gchar *params_string; /* one string later created from params[] */
  gchar *http_reply;
  gchar *access_token = NULL;

  timestamp = get_timestamp ();
  nonce = oauth_gen_nonce ();

  params[0] = g_strdup_printf ("oauth_verifier=%s", verifier);
  params[1] = g_strdup_printf ("oauth_consumer_key=%s", consumer_key);
  params[2] = g_strdup_printf ("oauth_nonce=%s", nonce);
  params[3] = g_strdup_printf ("oauth_signature_method=%s",
                              FLICKR_OAUTH_SIGNATURE_METHOD);
  params[4] = g_strdup_printf ("oauth_timestamp=%s", timestamp);
  params[5] = g_strdup_printf ("oauth_version=%s", FLICKR_OAUTH_VERSION);
  params[6] = g_strdup_printf ("oauth_token=%s", oauth_token);

  g_free (timestamp);
  g_free (nonce);

  signature = flickroauth_get_signature (consumer_secret,
                                         oauth_token_secret,
                                         FLICKR_OAUTH_ACCESSTOKEN_URL,
                                         params, 7);

  params[7] = g_strdup_printf ("oauth_signature=%s", signature);
  g_free (signature);

  params_string = oauth_serialize_url (8, 0, params);

  free_params (params, 8);

  url = g_strdup_printf ("%s?%s", FLICKR_OAUTH_ACCESSTOKEN_URL, params_string);
  g_free (params_string);

  http_reply = oauth_http_get2 (url, NULL, NULL);
  g_free (url);

  if (http_reply) {
    access_token = parse_access_token (http_reply, secret);
    g_free (http_reply);
  }

  return access_token;
}

gchar *
flickroauth_create_api_url (const gchar *consumer_key,
                            const gchar *consumer_secret,
                            const gchar *oauth_token,
                            const gchar *oauth_token_secret,
                            gchar **params,
                            const guint params_no)
{
  guint i;
  gchar *nonce;
  gchar *timestamp;
  gchar *signature;
  gchar *url;
  gchar *params_string;

  g_return_val_if_fail (consumer_key, NULL);

  /* handle Non-authorised call */
  if (oauth_token == NULL)
  {
    params_string = oauth_serialize_url (params_no, 0, params);

    url = g_strdup_printf ("%s?api_key=%s&%s", FLICKR_API_URL,
                                               consumer_key,
                                               params_string);

    g_free (params_string);

    return url;
  }

  /* there are 7 pre-filled parameters  in authorize call*/
  guint params_all_no = params_no + 7;
  gchar **params_all = g_malloc ((params_all_no) * sizeof (gchar *));

  if (params_all == NULL)
    return NULL;

  nonce = oauth_gen_nonce ();
  timestamp = get_timestamp ();

  params_all[0] = g_strdup_printf ("oauth_nonce=%s", nonce);
  params_all[1] = g_strdup_printf ("oauth_timestamp=%s", timestamp);
  params_all[2] = g_strdup_printf ("oauth_consumer_key=%s", consumer_key);
  params_all[3] = g_strdup_printf ("oauth_signature_method=%s",
                                                FLICKR_OAUTH_SIGNATURE_METHOD);
  params_all[4] = g_strdup_printf ("oauth_version=%s", FLICKR_OAUTH_VERSION);
  params_all[5] = g_strdup_printf ("oauth_token=%s", oauth_token);

  /* copy user parameters to the params_all */
  for (i = 0; i < params_no; i++)
    params_all[7 + i - 1] = g_strdup (params[i]);

  g_free (nonce);
  g_free (timestamp);

  signature = flickroauth_get_signature (consumer_secret,
                                         oauth_token_secret,
                                         FLICKR_API_URL, params_all,
                                         params_all_no - 1);

  params_all[params_all_no - 1] = g_strdup_printf ("oauth_signature=%s",
                                                   signature);
  g_free (signature);

  params_string = oauth_serialize_url (params_all_no, 0, params_all);

  free_params (params_all, params_all_no);
  g_free (params_all);

  url = g_strdup_printf ("%s?%s", FLICKR_API_URL, params_string);

  return url;
}

inline gchar *
flickroauth_authorization_url (const gchar *oauth_token, const gchar *perms)
{
  gchar *url;
  if (perms == NULL)
    url = g_strdup_printf ("%s?oauth_token=%s", FLICKR_OAUTH_AUTHPOINT,
                                              oauth_token);
  else
    url = g_strdup_printf ("%s?oauth_token=%s&perms=%s", FLICKR_OAUTH_AUTHPOINT,
                                                      oauth_token, perms);

  return url;
}

/* ---------- private functions ---------- */

inline static gchar *
get_timestamp (void)
{
  return g_strdup_printf ("%lu", g_get_real_time() / G_USEC_PER_SEC);
}

static gchar
*get_param_value (const gchar *string)
{
  gchar *eq = strchr (string, '=');

  if (eq == NULL)
    return NULL;

  return g_strdup (eq + 1);
}

static gchar *
parse_token (const gint parts_no, const gchar *string, gchar **secret)
{
  gchar **array;
  gchar *token = NULL;
  gint i = 0;

  array = g_strsplit (string, "&", parts_no);

  while (array[i] != NULL) {
    if (g_str_has_prefix (array[i], "oauth_token_secret"))
      (*secret) = get_param_value (array[i]);
    else if (g_str_has_prefix (array[i], "oauth_token"))
      token = get_param_value(array[i]);

    i++;
  }

  g_strfreev (array);

  return token;
}

inline static gchar *
parse_access_token (const gchar *string, gchar **secret)
{
  return parse_token(5, string, secret);
}

inline static gchar *
parse_request_token (const gchar *string, gchar **secret)
{
  return parse_token(3, string, secret);
}

static void
free_params (gchar **params, gint params_no)
{
  gint i;
  for (i = 0; i < params_no; i++)
    g_free (params[i]);
}
