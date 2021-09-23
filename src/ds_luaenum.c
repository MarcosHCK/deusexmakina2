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
#include <config.h>
#include <ds_luaenum.h>

#define _METATABLE "GEnumClass"

/*
 * Methods
 *
 */

void
luaD_pushenum(lua_State  *L,
              GType       g_type)
{
  GEnumClass** ptr = (GEnumClass**)
  lua_newuserdata(L, sizeof(GEnumClass*));
  luaL_setmetatable(L, _METATABLE);
  (*ptr) = g_type_class_ref(g_type);
}

gboolean
luaD_isenum(lua_State  *L,
            int         idx)
{
  GEnumClass **ptr =
  lua_touserdata(L, idx);

  if G_LIKELY(ptr != NULL)
  {
    if G_LIKELY
      (lua_getmetatable
       (L, idx) == TRUE)
    {
      luaL_getmetatable(L, _METATABLE);
      if G_LIKELY
        (lua_rawequal
         (L, -1, -2) == TRUE)
      {
        lua_pop(L, 2);
        return TRUE;
      }
      else
      {
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }
  }
return FALSE;
}

GEnumClass*
luaD_toenum(lua_State  *L,
            int         idx)
{
  GEnumClass **ptr =
  lua_touserdata(L, idx);

  if G_LIKELY(ptr != NULL)
  {
    if G_LIKELY
      (lua_getmetatable
       (L, idx) == TRUE)
    {
      luaL_getmetatable(L, _METATABLE);
      if G_LIKELY
        (lua_rawequal
         (L, -1, -2) == TRUE)
      {
        lua_pop(L, 2);
        return (*ptr);
      }
      else
      {
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }
  }
return NULL;
}

GEnumClass*
luaD_checkenum(lua_State *L,
               int        arg)
{
  gboolean is =
  luaD_isenum(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _ds_lua_typeerror(L, arg, _METATABLE);
  }
return luaD_toenum(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  GEnumClass* klass =
  luaD_checkenum(L, 1);

  lua_pushfstring
  (L,
   "%s (%p)",
   g_type_name_from_class((GTypeClass*) klass),
   (guintptr) klass);
return 1;
}

static int
__index(lua_State* L)
{
  GEnumClass* klass =
  luaD_checkenum(L, 1);
  const gchar* t;

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {

  }
return 0;
}

static int
__gc(lua_State* L)
{
  luaD_checkenum(L, 1);

  GEnumClass** ptr =
  lua_touserdata(L, 1);
  g_type_class_unref(*ptr);
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__index", __index},
  {"__gc", __gc},
  {NULL, NULL},
};

G_GNUC_INTERNAL
gboolean
_ds_luaenum_init(lua_State *L,
                 GError   **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  luaL_newmetatable(L, _METATABLE);
  luaL_setfuncs(L, instance_mt, 0);
  lua_pushliteral(L, "__name");
  lua_pushstring(L, _METATABLE);
  lua_settable(L, -3);
  lua_pop(L, 1);

_error_:
return success;
}

G_GNUC_INTERNAL
void
_ds_luaenum_fini(lua_State* L)
{
}
