/*
 * Copyright (C) 2010 Igalia S.L.
 * Copyright (C) 2011 Intel Corporation.
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

#include "grl-sync-priv.h"

void
grl_wait_for_async_operation_complete (GrlDataSync *ds)
{
  GMainLoop *ml;
  GMainContext *mc;

  ml = g_main_loop_new (g_main_context_get_thread_default (), TRUE);
  mc = g_main_loop_get_context (ml);

 while (!ds->complete) {
    g_main_context_iteration (mc, TRUE);
  }

  g_main_loop_unref (ml);
}

