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

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "grl-log"

#define GRL_DEFAULT_LOG_SPEC (G_LOG_LEVEL_WARNING |     \
                              G_LOG_LEVEL_CRITICAL |    \
                              G_LOG_LEVEL_ERROR)

static void
grl_log_handler (const gchar *domain,
                 GLogLevelFlags level,
                 const gchar *message,
                 gpointer user_data)
{
  GLogLevelFlags levels = GPOINTER_TO_INT (user_data);
  if (level & levels)
    g_log_default_handler (domain, level, message, NULL);
}

static const gchar *
get_domain_from_spec (const gchar *domain_spec)
{
  if (!g_ascii_strcasecmp ("default", domain_spec) ||
      !g_ascii_strcasecmp ("", domain_spec)) {
    return NULL; /* Default domain */
  } else {
    return domain_spec;
  }
}

static GLogLevelFlags
get_log_levels_from_spec (const gchar *level_spec)
{
  GLogLevelFlags levels = 0;

  if (!g_ascii_strcasecmp (level_spec, "-")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_ERROR;
  if (!g_ascii_strcasecmp (level_spec , "error")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_CRITICAL;
  if (!g_ascii_strcasecmp (level_spec , "critical")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_WARNING;
  if (!g_ascii_strcasecmp (level_spec , "warning")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_MESSAGE;
  if (!g_ascii_strcasecmp (level_spec , "message")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_INFO;
  if (!g_ascii_strcasecmp (level_spec , "info")) {
    return levels;
  }
  levels |= G_LOG_LEVEL_DEBUG;
  if (!g_ascii_strcasecmp (level_spec , "debug") ||
      !g_ascii_strcasecmp (level_spec , "*")) {
    return levels;
  }

  return levels;
}

static void
setup_log_domains (const gchar *domains)
{
  gchar **pairs;
  gchar **pair;
  gchar **pair_info ;
  gchar *domain_spec;
  gchar *level_spec;
  const gchar *domain;
  GLogLevelFlags levels;
  const gchar *env_log;

  /* Default logging policy */
  g_log_set_default_handler (grl_log_handler,
			     GINT_TO_POINTER (GRL_DEFAULT_LOG_SPEC));

  /* Now check user specs for specific domains, the user may change also the
     default set above by using "*" as domain name */

  /* First check environment for log configuration */
  env_log = g_getenv ("GRL_LOG");
  if (env_log) {
    domains = env_log;
    g_debug ("Using log configuration from $GRL_LOG");
  }

  pair = pairs = g_strsplit (domains, ",", 0);

  while (*pair) {
    pair_info = g_strsplit (*pair, ":", 2);
    if (pair_info[0] && pair_info[1]) {
      domain_spec = pair_info[0];
      level_spec = pair_info[1];
      g_debug ("domain: '%s', level: '%s'", domain_spec, level_spec);
      levels = get_log_levels_from_spec (level_spec);
      domain = get_domain_from_spec (domain_spec);
      if (!g_ascii_strcasecmp (domain, "*")) {
	g_log_set_default_handler (grl_log_handler, GINT_TO_POINTER (levels));
      } else {
	g_log_set_handler (domain,
			   G_LOG_LEVEL_MASK |
                           G_LOG_FLAG_RECURSION |
                           G_LOG_FLAG_FATAL,
			   grl_log_handler, GINT_TO_POINTER (levels));
      }
    } else {
      g_warning ("Invalid log spec: '%s'", *pair);
    }
    pair++;
  }
  g_strfreev (pairs);
}

void
grl_log_init (const gchar *domains)
{
  setup_log_domains (domains);
}
