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
#ifndef __LUAD_CLOSURE_INCLUDED__
#define __LUAD_CLOSURE_INCLUDED__ 1
#include <ds_export.h>
#include <girepository.h>
#include <glib-object.h>
#include <luad_core.h>

typedef struct _LClosure LClosure;

#if __LUAD_INSIDE__ == 1
#include <girepository.h>

struct _LClosure
{
  GClosure closure;
  GCallback address;
};

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
gboolean
_luaD_lclosure_init(lua_State* L, GError** error);
G_GNUC_INTERNAL
void
_luaD_lclosure_fini(lua_State* L);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LUAD_INSIDE__

#endif // __LUAD_CLOSURE_INCLUDED__
