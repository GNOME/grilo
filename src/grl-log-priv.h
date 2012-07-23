/*
 * Copyright (C) 2010 Intel Corporation
 *
 * Contact: Damien Lespiau <damien.lespiau@intel.com>
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

#ifndef _GRL_LOG_PRIV_H_
#define _GRL_LOG_PRIV_H_

#include <glib.h>

#include "grl-log.h"

G_BEGIN_DECLS

GRL_LOG_DOMAIN_EXTERN(log_log_domain);
GRL_LOG_DOMAIN_EXTERN(config_log_domain);
GRL_LOG_DOMAIN_EXTERN(data_log_domain);
GRL_LOG_DOMAIN_EXTERN(media_log_domain);
GRL_LOG_DOMAIN_EXTERN(plugin_log_domain);
GRL_LOG_DOMAIN_EXTERN(source_log_domain);
GRL_LOG_DOMAIN_EXTERN(multiple_log_domain);
GRL_LOG_DOMAIN_EXTERN(registry_log_domain);

void _grl_log_init_core_domains (void);
void _grl_log_free_core_domains (void);

G_END_DECLS

#endif /* _GRL_LOG_PRIV_H_ */
