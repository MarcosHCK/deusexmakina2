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
#ifndef __DS_LUAENUM_INCLUDED__
#define __DS_LUAENUM_INCLUDED__
#include <ds_lua.h>
#include <glib-object.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
gboolean
_ds_luaenum_init(lua_State *L,
                 GError   **error);
G_GNUC_INTERNAL
void
_ds_luaenum_fini(lua_State* L);

void
luaD_pushenum(lua_State  *L,
              GType       g_type);
gboolean
luaD_isenum(lua_State  *L,
            int         idx);
GEnumClass*
luaD_toenum(lua_State  *L,
            int         idx);
GEnumClass*
luaD_checkenum(lua_State *L,
               int        arg);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUAENUM_INCLUDED__