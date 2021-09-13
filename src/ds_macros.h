/*  Copyright 2021-2022 MarcosHCK
 *  This file is part of deusexmakina2.
 *
 *  deusexmakina2 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  deusexmakina2 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with deusexmakina2.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __DS_MACROS_INCLUDED__
#define __DS_MACROS_INCLUDED__ 1
#define __GLIB_H_INSIDE__ 1
#include <glib/gmacros.h>

#define goto_error() \
G_STMT_START { \
  g_warn_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, "goto_error"); \
  success = FALSE; \
  goto _error_; \
} G_STMT_END

static inline guint
ds_steal_handle_id (gpointer pp)
{
  guint *ptr = (guint *) pp;
  guint ref;

  ref = *ptr;
  *ptr = 0;
return ref;
}

/* type safety */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)) && !defined(__cplusplus) && GLIB_VERSION_MAX_ALLOWED >= GLIB_VERSION_2_58
#define ds_steal_handle_id(pp) ((__typeof__(*pp)) (ds_steal_handle_id) (pp))
#else  /* __GNUC__ */
/* This version does not depend on gcc extensions, but gcc does not warn
 * about incompatible-pointer-types: */
#define ds_steal_handle_id(pp) \
  (0 ? (*(pp)) : (ds_steal_handle_id) (pp))
#endif /* __GNUC__ */

#endif // __DS_MACROS_INCLUDED__
