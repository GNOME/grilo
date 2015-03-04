/*
 * Copyright (C) 2010, 2011 Igalia S.L.
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

#if !defined (_GRILO_H_INSIDE_) && !defined (GRILO_COMPILATION)
#error "Only <grilo.h> can be included directly."
#endif

#ifndef _GRL_ERROR_H_
#define _GRL_ERROR_H_

#define GRL_CORE_ERROR g_quark_from_static_string("grilo.error.general")

/**
 * GrlCoreError:
 * @GRL_CORE_ERROR_BROWSE_FAILED: The browse operation failed
 * @GRL_CORE_ERROR_SEARCH_FAILED: The search operation failed
 * @GRL_CORE_ERROR_SEARCH_NULL_UNSUPPORTED: Searching NULL-text is not supported
 * @GRL_CORE_ERROR_QUERY_FAILED: The query operation failed
 * @GRL_CORE_ERROR_RESOLVE_FAILED: The resolution operation failed
 * @GRL_CORE_ERROR_MEDIA_NOT_FOUND: The media was not found
 * @GRL_CORE_ERROR_STORE_FAILED: The store operation failed
 * @GRL_CORE_ERROR_STORE_METADATA_FAILED: The store metadata operation failed
 * @GRL_CORE_ERROR_REMOVE_FAILED: The removal operation failed
 * @GRL_CORE_ERROR_MEDIA_FROM_URI_FAILED: The media from_uri operation failed
 * @GRL_CORE_ERROR_CONFIG_LOAD_FAILED: Failed to load plugin configuration from a file
 * @GRL_CORE_ERROR_CONFIG_FAILED: Failed to set configuration for plugin
 * @GRL_CORE_ERROR_UNREGISTER_SOURCE_FAILED: Failed to unregister source
 * @GRL_CORE_ERROR_LOAD_PLUGIN_FAILED: Failed to load plugin
 * @GRL_CORE_ERROR_UNLOAD_PLUGIN_FAILED: Failed to unload plugin
 * @GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED: Failed to register metadata key
 * @GRL_CORE_ERROR_NOTIFY_CHANGED_FAILED: Failed to start changed notifications
 * @GRL_CORE_ERROR_OPERATION_CANCELLED: The operation was cancelled
 * @GRL_CORE_ERROR_AUTHENTICATION_TOKEN: Invalid authentication token
 *
 * These constants identify all the available core errors
 */
typedef enum {
  GRL_CORE_ERROR_BROWSE_FAILED = 1,
  GRL_CORE_ERROR_SEARCH_FAILED,
  GRL_CORE_ERROR_SEARCH_NULL_UNSUPPORTED,
  GRL_CORE_ERROR_QUERY_FAILED,
  GRL_CORE_ERROR_RESOLVE_FAILED,
  GRL_CORE_ERROR_MEDIA_NOT_FOUND,
  GRL_CORE_ERROR_STORE_FAILED,
  GRL_CORE_ERROR_STORE_METADATA_FAILED,
  GRL_CORE_ERROR_REMOVE_FAILED,
  GRL_CORE_ERROR_MEDIA_FROM_URI_FAILED,
  GRL_CORE_ERROR_CONFIG_LOAD_FAILED,
  GRL_CORE_ERROR_CONFIG_FAILED,
  GRL_CORE_ERROR_UNREGISTER_SOURCE_FAILED,
  GRL_CORE_ERROR_LOAD_PLUGIN_FAILED,
  GRL_CORE_ERROR_UNLOAD_PLUGIN_FAILED,
  GRL_CORE_ERROR_REGISTER_METADATA_KEY_FAILED,
  GRL_CORE_ERROR_NOTIFY_CHANGED_FAILED,
  GRL_CORE_ERROR_OPERATION_CANCELLED,
  GRL_CORE_ERROR_AUTHENTICATION_TOKEN
} GrlCoreError;

#endif /* _GRL_ERROR_H_ */
