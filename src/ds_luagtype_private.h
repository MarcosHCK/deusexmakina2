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
#ifndef __DS_LUAGTYPE_PRIVATE_INCLUDED__
#define __DS_LUAGTYPE_PRIVATE_INCLUDED__
#include <ds_luagtype.h>

typedef struct _DsGType DsGType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _DsGType
{
  GType g_type;
  union
  {
    gpointer thing;
    gpointer boxed;
    GTypeClass* klass;
    GTypeInterface* iface;
    GTypeInstance* instance;
    gint enum_value;
    guint flags_value;
  };

  GDestroyNotify notify;
  guint fundamental : 1;
  guint classed : 1;
  guint instantiable : 1;
  guint instanced : 1;
};

G_GNUC_INTERNAL
gboolean
_ds_tovalue(lua_State* L, gint idx, GValue* value, GType g_type, GError** error);
G_GNUC_INTERNAL
gboolean
_ds_pushvalue(lua_State* L, GValue* value, GError** error);

G_GNUC_INTERNAL
DsGType*
_luaD_pushgtype(lua_State* L, GType g_type, gpointer thing);
G_GNUC_INTERNAL
gboolean
_luaD_isgtype(lua_State* L, int idx);
G_GNUC_INTERNAL
DsGType*
_luaD_togtype(lua_State* L, int idx);
G_GNUC_INTERNAL
DsGType*
_luaD_checkgtype(lua_State* L, int arg);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUAGTYPE_PRIVATE_INCLUDED__
