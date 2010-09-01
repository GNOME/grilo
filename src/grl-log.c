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

#include "grl-log.h"
#include "grl-log-priv.h"

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

struct _GrlLogDomain {
  /*< private >*/
  GrlLogLevel log_level;
  char *name;
};


static gchar **grl_log_env;          /* 'domain:level' array from GRL_LOG */

static GrlLogLevel grl_default_log_level = GRL_LOG_LEVEL_WARNING;
static GSList *log_domains = NULL;  /* the list of GrlLogDomain's */

/* Catch all log domain */
GRL_LOG_DOMAIN(GRL_LOG_DOMAIN_DEFAULT);

#define GRL_LOG_DOMAIN_DEFAULT  log_log_domain
GRL_LOG_DOMAIN(log_log_domain);

static GrlLogDomain *
grl_log_domain_find_by_name (const gchar *name)
{
  GSList *list;

  for (list = log_domains; list; list = g_slist_next (list)) {
    GrlLogDomain *log_domain = list->data;

    if (g_strcmp0 (log_domain->name, name) == 0)
      return log_domain;
  }

  return NULL;
}

static GrlLogDomain *
_grl_log_domain_new_internal (const gchar *name)
{
  GrlLogDomain *domain;

  if (*name == '\0')
    return GRL_LOG_DOMAIN_DEFAULT;

  domain = g_slice_new (GrlLogDomain);
  domain->log_level = grl_default_log_level;
  domain->name = g_strdup (name);

  log_domains = g_slist_prepend (log_domains, domain);

  return domain;
}

GrlLogDomain *
grl_log_domain_new (const gchar *name)
{
  GrlLogDomain *domain;
  gchar **pair;

  g_return_val_if_fail (name, NULL);

  domain = _grl_log_domain_new_internal (name);

  /* If the GRL_LOG env variable contains @name, let's override that domain
   * verbosity */
  if (grl_log_env == NULL)
    return domain;

  pair = grl_log_env;

  while (*pair) {
    gchar **pair_info;
    gchar *domain_spec;

    pair_info = g_strsplit (*pair, ":", 2);
    domain_spec = pair_info[0];

    if (g_strcmp0 (domain_spec, name) == 0)
      grl_log_init (*pair);

    g_strfreev (pair_info);
    pair++;
  }

  return domain;
}

static void
_grl_log_domain_free_internal (GrlLogDomain *domain)
{
  log_domains = g_slist_remove (log_domains, domain);
  g_free (domain->name);
  g_slice_free (GrlLogDomain, domain);
}

void
grl_log_domain_free (GrlLogDomain *domain)
{
  g_return_if_fail (domain);

  /* domain can actually be GRL_LOG_DOMAIN_DEFAULT if the domain name given
   * in _new() was "", freeing the default domain is not possible from the
   * public API */
  if (domain == GRL_LOG_DOMAIN_DEFAULT)
    return;

  _grl_log_domain_free_internal (domain);
}

static void
grl_log_domain_set_level_all (GrlLogLevel level)
{
  GSList *list;

  /* Set the default log level to be level, so newly created domains will
   * have the correct level */
  grl_default_log_level = level;

  for (list = log_domains; list; list = g_slist_next (list)) {
    GrlLogDomain *log_domain = list->data;

    log_domain->log_level = level;
  }
}

static GrlLogDomain *
get_domain_from_spec (const gchar *domain_spec)
{
  GrlLogDomain *domain;

  domain = grl_log_domain_find_by_name (domain_spec);

  return domain;
}

static gchar *name2level[GRL_LOG_LEVEL_LAST] = {
  "none", "error", "warning", "message", "info", "debug"
};

static GrlLogLevel
get_log_level_from_spec (const gchar *level_spec)
{
  guint i;
  long int level_num;
  char *tail;

  /* "-" or "none" (from name2level) can be used to disable all logging */
  if (strcmp (level_spec, "-") == 0) {
    return GRL_LOG_LEVEL_NONE;
  }

  /* '*' means everything */
  if (strcmp (level_spec, "*") == 0) {
    return GRL_LOG_LEVEL_LAST - 1;
  }

  errno = 0;
  level_num = strtol (level_spec, &tail, 0);
  if (!errno
      && tail != level_spec
      && level_num >= GRL_LOG_LEVEL_NONE
      && level_num <= GRL_LOG_LEVEL_LAST - 1)
      return (GrlLogLevel) level_num;

  for (i = 0; i < GRL_LOG_LEVEL_LAST; i++)
    if (strcmp (level_spec, name2level[i]) == 0)
      return i;

  /* If the spec does not match one of our levels, just return the current
   * default log level */
  return grl_default_log_level;
}

static void
setup_log_domains (const gchar *domains)
{
  gchar **pairs;
  gchar **pair;
  gchar **pair_info ;
  gchar *domain_spec;
  gchar *level_spec;
  GrlLogDomain *domain;
  GrlLogLevel level;

  pair = pairs = g_strsplit (domains, ",", 0);

  while (*pair) {
    pair_info = g_strsplit (*pair, ":", 2);
    if (pair_info[0] && pair_info[1]) {
      domain_spec = pair_info[0];
      level_spec = pair_info[1];

      level = get_log_level_from_spec (level_spec);
      domain = get_domain_from_spec (domain_spec);

      if (strcmp (domain_spec, "*") == 0)
        grl_log_domain_set_level_all (level);

      if (domain == NULL) {
       g_strfreev (pair_info);
       pair++;
       continue;
      }

      domain->log_level = level;

      GRL_DEBUG ("domain: '%s', level: '%s'", domain_spec, level_spec);

      g_strfreev (pair_info);
    } else {
      GRL_WARNING ("Invalid log spec: '%s'", *pair);
    }
    pair++;
  }
  g_strfreev (pairs);
}

static void
grl_log_valist (GrlLogDomain *domain,
                GrlLogLevel   level,
                const gchar  *strloc,
                const gchar  *format,
                va_list       args)
{
  gchar *message;
  GLogLevelFlags level2flag[GRL_LOG_LEVEL_LAST] = {
    0, G_LOG_LEVEL_CRITICAL, G_LOG_LEVEL_WARNING, G_LOG_LEVEL_MESSAGE,
    G_LOG_LEVEL_INFO, G_LOG_LEVEL_DEBUG
  };

  g_return_if_fail (domain);
  g_return_if_fail (level > 0 && level < GRL_LOG_LEVEL_LAST);
  g_return_if_fail (strloc);
  g_return_if_fail (format);

  message = g_strdup_vprintf (format, args);

  if (level <= domain->log_level)
    g_log (G_LOG_DOMAIN, level2flag[level],
           "[%s] %s: %s", domain->name, strloc, message);

  g_free (message);
}

void
grl_log (GrlLogDomain *domain,
         GrlLogLevel   level,
         const gchar  *strloc,
         const gchar  *format,
         ...)
{
  va_list var_args;

  va_start (var_args, format);
  grl_log_valist (domain, level, strloc, format, var_args);
  va_end (var_args);
}

#define DOMAIN_INIT(domain, name) G_STMT_START {  \
    domain = _grl_log_domain_new_internal (name); \
} G_STMT_END

void
_grl_log_init_core_domains (void)
{
  const gchar *log_env;

  DOMAIN_INIT (GRL_LOG_DOMAIN_DEFAULT, "");
  DOMAIN_INIT (log_log_domain, "log");
  DOMAIN_INIT (config_log_domain, "config");
  DOMAIN_INIT (media_log_domain, "media");
  DOMAIN_INIT (media_plugin_log_domain, "media-plugin");
  DOMAIN_INIT (media_source_log_domain, "media-source");
  DOMAIN_INIT (metadata_source_log_domain, "metadata-source");
  DOMAIN_INIT (multiple_log_domain, "multiple");
  DOMAIN_INIT (plugin_registry_log_domain, "plugin-registry");

  /* Retrieve the GRL_LOG environment variable, initialize core domains from
   * it if applicable and keep it for grl_log_domain_new(). Plugins are using
   * grl_log_domain_new() in their init() functions to initialize their log
   * domains. At that time, we'll look at the saved GRL_LOG to overrive the
   * verbosity */
  log_env = g_getenv ("GRL_LOG");
  if (log_env) {
    GRL_DEBUG ("Using log configuration from GRL_LOG: %s", log_env);
    setup_log_domains (log_env);
    grl_log_env = g_strsplit (log_env, ",", 0);
  }

}

#undef DOMAIN_INIT

#define DOMAIN_FREE(domain) G_STMT_START {  \
    _grl_log_domain_free_internal (domain);   \
} G_STMT_END

void
_grl_log_free_core_domains (void)
{
  DOMAIN_FREE (GRL_LOG_DOMAIN_DEFAULT);
  DOMAIN_FREE (log_log_domain);
  DOMAIN_FREE (config_log_domain);
  DOMAIN_FREE (media_log_domain);
  DOMAIN_FREE (media_plugin_log_domain);
  DOMAIN_FREE (media_source_log_domain);
  DOMAIN_FREE (metadata_source_log_domain);
  DOMAIN_FREE (multiple_log_domain);
  DOMAIN_FREE (plugin_registry_log_domain);

  g_strfreev (grl_log_env);
}

#undef DOMAIN_FREE

void
grl_log_init (const gchar *domains)
{
  setup_log_domains (domains);
}
