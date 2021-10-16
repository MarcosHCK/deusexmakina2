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
#ifndef __DS_LUAGTYPE_INCLUDED__
#define __DS_LUAGTYPE_INCLUDED__
#include <ds_lua.h>
#include <glib-object.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
gboolean
_ds_luagtype_init(lua_State* L, GError** error);
G_GNUC_INTERNAL
void
_ds_luagtype_fini(lua_State* L);

void
luaD_pushobject(lua_State* L, GObject* object);
gboolean
luaD_isobject(lua_State* L, int idx);
GObject*
luaD_toobject(lua_State* L, int idx);
GObject*
luaD_checkobject(lua_State* L, int idx);

#define luaD_pushobject(L, object) \
  (luaD_pushobject((L), ((GObject*) (object))))
#define luaD_isobject(L, narg) \
  ((gboolean) luaD_isobject((L), (narg)))
#define luaD_toobject(L, narg) \
  ((gpointer) luaD_toobject((L), (narg)))
#define luaD_checkobject(L, narg) \
  ((gpointer) luaD_checkobject((L), (narg)))

void
luaD_pushclass(lua_State* L, GType g_type);
gboolean
luaD_isclass(lua_State* L, int idx);
GObjectClass*
luaD_toclass(lua_State* L, int idx);
GObjectClass*
luaD_checkclass(lua_State* L, int idx);

#define luaD_pushclass(L, g_type) \
  (luaD_pushclass((L), (g_type)))
#define luaD_isclass(L, narg) \
  ((gboolean) luaD_isclass((L), (narg)))
#define luaD_toclass(L, narg) \
  ((gpointer) luaD_toclass((L), (narg)))
#define luaD_checkclass(L, narg) \
  ((gpointer) luaD_checkclass((L), (narg)))

void
luaD_pushboxed(lua_State* L, GType g_type, gpointer instance);
gboolean
luaD_isboxed(lua_State* L, int idx);
gpointer
luaD_toboxed(lua_State* L, int idx, GType* g_type);
gpointer
luaD_checkboxed(lua_State* L, int idx, GType* g_type);

void
luaD_pushenum(lua_State* L, GType g_type, gint value);
gboolean
luaD_isenum(lua_State* L, int idx);
gint
luaD_toenum(lua_State* L, int idx, GType* g_type);
gint
luaD_checkenum(lua_State* L, int idx, GType* g_type);

void
luaD_pushflags(lua_State* L, GType g_type, guint value);
gboolean
luaD_isflags(lua_State* L, int idx);
guint
luaD_toflags(lua_State* L, int idx, GType* g_type);
guint
luaD_checkflags(lua_State* L, int idx, GType* g_type);

void
luaD_pushgtype(lua_State* L, GType g_type);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUAGTYPE_INCLUDED__
