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

/**
 * SECTION:grl-util
 * @short_description: utility functions
 */

#include "grl-util.h"

/**
 * grl_paging_translate:
 * @skip: number of elements to skip
 * @count: number of elements to retrieve
 * @max_page_size: maximum value for page size
 * @page_size: optimal page size
 * @page_number: page which contain the first element to retrieve (starting at 1)
 * @internal_offset: in the @page_number, offset where first element can be found (starting at 0)
 *
 * Grilo browsing implements a paging mechanism through @skip and @count values.
 *
 * But there are some services (like Jamendo or Flickr) where paging is done
 * through a page number and page size: user request all elements in a page,
 * specifying in most cases what is the page size.
 *
 * This function is a helper for this task, computing from @skip and @count what
 * is the optimal value of page size (limited by @max_page_size), which page
 * should the user request, and where requested data start inside the page.
 *
 * By optimal we mean that it computes those values so only one page is required
 * to satisfy the data, using the smallest page size. If user is limiting page
 * size, then more requests to services might be needed. But still page size
 * will be an optimal value.
 *
 * Since: 0.1.6
 **/
void grl_paging_translate (guint skip,
                           guint count,
                           guint max_page_size,
                           guint *page_size,
                           guint *page_number,
                           guint *internal_offset)
{
  gulong _page_size;
  gulong last_element;

  if (skip < count) {
    _page_size = skip + count;
    if (max_page_size > 0) {
      _page_size = CLAMP (_page_size, 0, max_page_size);
    }
  } else {
    _page_size = count;
    last_element = skip + count - 1;
    while (skip/_page_size != last_element/_page_size &&
           (max_page_size == 0 || _page_size < max_page_size)) {
      _page_size++;
    }
  }
  _page_size = CLAMP (_page_size, 0, G_MAXUINT);

  if (page_size) {
    *page_size = _page_size;
  }

  if (page_number) {
    *page_number = skip/_page_size + 1;
  }

  if (internal_offset) {
    *internal_offset = skip%_page_size;
  }
}

/**
 * grl_list_from_va: (skip)
 * @p: first pointer
 * @...: va_list pointers
 *
 * Returns a #GList containing the va_list pointers. Use @NULL to finalize them,
 *
 * Returns: a #GList.
 *
 * Since: 0.1.6
 **/
GList *
grl_list_from_va (gpointer p, ...)
{
  GList *pointer_list = NULL;
  gpointer next_pointer;
  va_list va_pointers;

  va_start (va_pointers, p);
  next_pointer = p;
  while (next_pointer) {
    pointer_list = g_list_prepend (pointer_list, next_pointer);
    next_pointer = va_arg (va_pointers, gpointer);
  }
  va_end (va_pointers);

  return g_list_reverse (pointer_list);
}

/**
 * grl_date_time_from_iso8601:
 * @date: a date expressed in iso8601 format
 *
 * Returns: a newly-allocated #GDateTime set to the time corresponding to
 * @date, or %NULL if @date could not be parsed properly.
 *
 * Since: 0.2.0
 */
GDateTime *
grl_date_time_from_iso8601 (const gchar *date)
{
  GTimeVal t = { 0, };
  gboolean ret;

  ret = g_time_val_from_iso8601 (date, &t);

  /* second condition works around
   * https://bugzilla.gnome.org/show_bug.cgi?id=650968 */
  if (!ret || (t.tv_sec == 0 && t.tv_usec == 0)) {
    /* We might be in the case where there is a date alone. In that case, we
     * take the convention of setting it to noon GMT */
    gchar *date_time = g_strdup_printf ("%sT12:00:00Z", date);
    ret = g_time_val_from_iso8601 (date_time, &t);
    g_free (date_time);
  }

  if (ret)
    return g_date_time_new_from_timeval_utc (&t);

  return NULL;
}

