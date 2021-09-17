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
#include <ds_luaobj.h>

#define _METATABLE "GObject"

/*
 * Methods
 *
 */

gboolean
_ds_tovalue (lua_State* L,
             gint idx,
             GValue* value,
             GType g_type,
             GError** error);

void
luaD_tovalue(lua_State* L,
             gint idx,
             GValue* value,
             GType g_type)
{
  GError* tmp_err = NULL;
  _ds_tovalue(L, idx, value, g_type, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    lua_pushstring(L, tmp_err->message);
    g_error_free(tmp_err);
    lua_error(L);
  }
}

void
luaD_pushobject(lua_State  *L,
                GObject    *obj)
{
  GObject** ptr = (GObject**)
  lua_newuserdata(L, sizeof(GObject*));
  luaL_setmetatable(L, _METATABLE);
  (*ptr) = g_object_ref(obj);
}

/* Taken as-in from LuaJIT code */
static int
_typeerror(lua_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  if (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  else if (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  else
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  return luaL_argerror(L, arg, msg);
}

GObject*
luaD_toobject(lua_State  *L,
              int         idx)
{
  GObject **ptr =
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

GObject*
luaD_checkobject(lua_State  *L,
                 int         arg)
{
  GObject* obj =
  luaD_toobject(L, arg);
  if G_UNLIKELY(obj == NULL)
  {
    _typeerror(L, arg, _METATABLE);
  }
return obj;
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  GObject* obj =
  luaD_checkobject(L, 1);

  lua_pushfstring
  (L,
   "%s (%p)",
   g_type_name_from_instance((GTypeInstance*)obj),
   (guintptr) obj);
return 1;
}

static int
__index_ref(lua_State* L)
{
  GObject* obj =
  luaD_checkobject(L, 1);
  g_object_ref(obj);
return 1;
}

static int
__index_unref(lua_State* L)
{
  GObject* obj =
  luaD_checkobject(L, 1);
  g_object_unref(obj);
return 0;
}

static int
__index(lua_State* L)
{
  const gchar* t;

  if G_LIKELY
    (lua_isstring(L, 1)
     && (t = lua_tostring(L, 1)) != NULL)
  {
    if(!strcmp(t, "ref"))
    {
      lua_pushcfunction(L, __index_ref);
      return 1;
    } else
    if(!strcmp(t, "unref"))
    {
      lua_pushcfunction(L, __index_unref);
      return 1;
    }
  }
return 0;
}

static int
__gc(lua_State* L)
{
  luaD_checkobject(L, 1);

  GObject** ptr =
  lua_touserdata(L, 1);
  g_clear_object(ptr);
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

gboolean
_ds_luaobj_init(lua_State  *L,
                GError    **error)
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

void
_ds_luaobj_fini(lua_State* L)
{
  g_print("fini\r\n");
}