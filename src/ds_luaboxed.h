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
#ifndef __DS_LUABOXED_INCLUDED__
#define __DS_LUABOXED_INCLUDED__
#include <ds_lua.h>
#include <glib-object.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
gboolean
_ds_luaboxed_init(lua_State  *L,
                  GError    **error);
G_GNUC_INTERNAL
void
_ds_luaboxed_fini(lua_State* L);

void
luaD_pushboxed(lua_State *L,
               GType      g_type,
               gpointer   boxed);
gboolean
luaD_isboxed(lua_State  *L,
             int         idx);
gpointer
luaD_toboxed(lua_State  *L,
             int         idx,
             GType      *g_type);
gpointer
luaD_checkboxed(lua_State *L,
                int        arg,
                GType     *g_type);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUABOXED_INCLUDED__
