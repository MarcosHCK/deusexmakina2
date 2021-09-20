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
#ifndef __OVERWRITES_INCLUDED__
#define __OVERWRITES_INCLUDED__
#define __INSIDE_DYNASM_FILE__
#include <glib.h>

#define DASM_FDEF G_GNUC_INTERNAL

#define DASM_EXTERN(ctx, addr, idx, type) 0

#define DASM_M_GROW(ctx, t, p, sz, need) \
  G_STMT_START { \
    size_t _sz = (sz), _need = (need); \
    if (_sz < _need) { \
      if (_sz < 16) _sz = 16; \
      while (_sz < _need) _sz += _sz; \
      (p) = (t *)g_realloc((p), _sz); \
      if ((p) == NULL) exit(1); \
      (sz) = _sz; \
    } \
  } G_STMT_END

#define DASM_M_FREE(ctx, p, sz) g_free(p);

#endif // __OVERWRITES_INCLUDED__
