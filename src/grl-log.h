/*
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *                    2003 Benjamin Otte <in7y118@public.uni-hamburg.de>
 *                    2010 Igalia S.L.
 *                    2010 Intel Corporation
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
 * Part of this code has been adapted from GStreamer gst/gstinfo.h
 */

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_LOG_H_
#define _GRL_LOG_H_

#include <glib.h>

G_BEGIN_DECLS

/**
 * GrlLogLevel:
 * @GRL_LOG_LEVEL_NONE: Log level none
 * @GRL_LOG_LEVEL_ERROR: Log on error
 * @GRL_LOG_LEVEL_WARNING: Log on warning
 * @GRL_LOG_LEVEL_MESSAGE: Log on message
 * @GRL_LOG_LEVEL_INFO: Log on info
 * @GRL_LOG_LEVEL_DEBUG: Log on debug
 * @GRL_LOG_LEVEL_LAST: Last level of log
 *
 * Grilo log levels. Defines the level of verbosity selected in Grilo.
 */
typedef enum {
  GRL_LOG_LEVEL_NONE,
  GRL_LOG_LEVEL_ERROR,
  GRL_LOG_LEVEL_WARNING,
  GRL_LOG_LEVEL_MESSAGE,
  GRL_LOG_LEVEL_INFO,
  GRL_LOG_LEVEL_DEBUG,

  GRL_LOG_LEVEL_LAST
} GrlLogLevel;

/* Opaque type */
typedef struct _GrlLogDomain GrlLogDomain;

extern GrlLogDomain *GRL_LOG_DOMAIN_DEFAULT;

/**
 * GRL_LOG_DOMAIN:
 * @domain: the log domain
 *
 * Defines a GrlLogDomain variable.
 */
#define GRL_LOG_DOMAIN(domain) GrlLogDomain *domain = NULL

/**
 * GRL_LOG_DOMAIN_EXTERN:
 * @domain: the log domain
 *
 * Declares a GrlLogDomain variable as extern. Use in header files.
 */
#define GRL_LOG_DOMAIN_EXTERN(domain) extern GrlLogDomain *domain

/**
 * GRL_LOG_DOMAIN_STATIC:
 * @domain: the log domain
 *
 * Defines a static GrlLogDomain variable.
 */
#define GRL_LOG_DOMAIN_STATIC(domain) static GrlLogDomain *domain = NULL

/**
 * GRL_LOG_DOMAIN_INIT:
 * @domain: the log domain to initialize.
 * @name: the name of the log domain.
 *
 * Creates a new #GrlLogDomain with the given name.
 */
#define GRL_LOG_DOMAIN_INIT(domain, name) G_STMT_START { \
  if (domain == NULL)                                    \
    domain = grl_log_domain_new (name);                  \
} G_STMT_END

/**
 * GRL_LOG_DOMAIN_FREE:
 * @domain: the log domain to free.
 *
 * Free a previously allocated #GrlLogDomain.
 */
#define GRL_LOG_DOMAIN_FREE(domain) G_STMT_START {  \
  grl_log_domain_free (domain);                     \
  domain = NULL;                                    \
} G_STMT_END

/**
 * GRL_LOG:
 * @domain: the log domain to use
 * @level: the severity of the message
 * @...: A printf-style message to output
 *
 * Outputs a debugging message. This is the most general macro for outputting
 * debugging messages. You will probably want to use one of the ones described
 * below.
 */
#ifdef G_HAVE_ISO_VARARGS

#define GRL_LOG(domain, level, ...) G_STMT_START{       \
    grl_log ((domain), (level), G_STRLOC, __VA_ARGS__); \
}G_STMT_END

#elif G_HAVE_GNUC_VARARGS

#define GRL_LOG(domain, level, args...) G_STMT_START{ \
    grl_log ((domain), (level), G_STRLOC, ##args);    \
}G_STMT_END

#else /* no variadic macros, use inline */

static inline void
GRL_LOG_valist (GrlLogDomain *domain,
                GrlLogLevel   level,
                const char   *format,
                va_list       varargs)
{
  grl_log (domain, level, "", format, varargs);
}

static inline void
GRL_LOG (GrlLogDomain *domain,
         GrlLogLevel   level,
         const char   *format,
         ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_DOMAIN_LOG_valist (domain, level, format, varargs);
  va_end (varargs);
}

#endif /* G_HAVE_ISO_VARARGS */

/**
 * GRL_ERROR:
 * @...: printf-style message to output
 *
 * Output an error message in the default log domain.
 */
/**
 * GRL_WARNING:
 * @...: printf-style message to output
 *
 * Output a warning message in the default log domain.
 */
/**
 * GRL_MESSAGE:
 * @...: printf-style message to output
 *
 * Output a logging message in the default log domain.
 */
/**
 * GRL_INFO:
 * @...: printf-style message to output
 *
 * Output an informational message in the default log domain.
 */
/**
 * GRL_DEBUG:
 * @...: printf-style message to output
 *
 * Output a debugging message in the default log domain.
 */

#if G_HAVE_ISO_VARARGS

#define GRL_ERROR(...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_ERROR, __VA_ARGS__)
#define GRL_WARNING(...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_WARNING, __VA_ARGS__)
#define GRL_MESSAGE(...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_MESSAGE, __VA_ARGS__)
#define GRL_INFO(...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_INFO, __VA_ARGS__)
#define GRL_DEBUG(...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_DEBUG, __VA_ARGS__)

#elif G_HAVE_GNUC_VARARGS

#define GRL_ERROR(args...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_ERROR, ##args)
#define GRL_WARNING(args...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_WARNING, ##args)
#define GRL_MESSAGE(args...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_MESSAGE, ##args)
#define GRL_INFO(args...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_INFO, ##args)
#define GRL_DEBUG(args...) \
  GRL_LOG (GRL_LOG_DOMAIN_DEFAULT, GRL_LOG_LEVEL_DEBUG, ##args)

#else /* no variadic macros, use inline */

static inline void
GRL_ERROR (GrlLogDomain *domain, const char *format, ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_valist (domain, GRL_LOG_LEVEL_ERROR, format, varargs);
  va_end (varargs);
}

static inline void
GRL_WARNING (GrlLogDomain *domain, const char *format, ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_valist (domain, GRL_LOG_LEVEL_WARNING, format, varargs);
  va_end (varargs);
}

static inline void
GRL_MESSAGE (GrlLogDomain *domain, const char *format, ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_valist (domain, GRL_LOG_LEVEL_MESSAGE, format, varargs);
  va_end (varargs);
}

static inline void
GRL_INFO (GrlLogDomain *domain, const char *format, ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_valist (domain, GRL_LOG_LEVEL_INFO, format, varargs);
  va_end (varargs);
}

static inline void
GRL_DEBUG (GrlLogDomain *domain, const char *format, ...)
{
  va_list varargs;

  va_start (varargs, format);
  GRL_LOG_valist (domain, GRL_LOG_LEVEL_DEBUG, format, varargs);
  va_end (varargs);
}

#endif /* G_HAVE_ISO_VARARGS */

GrlLogDomain *  grl_log_domain_new    (const gchar *name);
void            grl_log_domain_free   (GrlLogDomain *domain);

void            grl_log_configure     (const gchar  *config);
void            grl_log               (GrlLogDomain *domain,
                                       GrlLogLevel   level,
                                       const gchar  *strloc,
                                       const gchar  *format,
                                       ...) G_GNUC_PRINTF (4, 5) G_GNUC_NO_INSTRUMENT;

G_END_DECLS

#endif /* _GRL_LOG_H_ */
