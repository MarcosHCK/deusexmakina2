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
#ifndef __DS_LUAOBJ_INCLUDED__
#define __DS_LUAOBJ_INCLUDED__
#include <ds_lua.h>
#include <glib-object.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

gboolean
_ds_luaobj_init(lua_State  *L,
                GError    **error);
void
_ds_luaobj_fini(lua_State* L);

void
luaD_tovalue(lua_State* L,
             gint idx,
             GValue* value,
             GType g_type);

void
luaD_pushobject(lua_State  *L,
                GObject    *obj);
gboolean
luaD_isobject(lua_State  *L,
              int         idx);
GObject*
luaD_toobject(lua_State  *L,
              int         idx);
GObject*
luaD_checkobject(lua_State  *L,
                 int         arg);

#define luaD_pushobject(L, object) \
  (luaD_pushobject((L), ((GObject*) (object))))
#define luaD_toobject(L, narg) \
  ((gpointer) luaD_toobject((L), (narg)))
#define luaD_checkobject(L, narg) \
  ((gpointer) luaD_checkobject((L), (narg)))

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUAOBJ_INCLUDED__
