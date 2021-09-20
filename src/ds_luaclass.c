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
#include <ds_callable.h>
#include <ds_luaclass.h>
#include <ds_luaclosure.h>

#define _METATABLE "GObjectClass"

/*
 * Methods
 *
 */

void
luaD_pushclass(lua_State *L,
               GType      g_type)
{
  GObjectClass** ptr = (GObjectClass**)
  lua_newuserdata(L, sizeof(GObjectClass*));
  luaL_setmetatable(L, _METATABLE);
  (*ptr) = g_type_class_ref(g_type);
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

gboolean
luaD_isclass(lua_State  *L,
             int         idx)
{
  GObjectClass **ptr =
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

GObjectClass*
luaD_toclass(lua_State  *L,
             int         idx)
{
  GObjectClass **ptr =
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

GObjectClass*
luaD_checkclass(lua_State  *L,
                int         arg)
{
  gboolean is =
  luaD_isclass(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _typeerror(L, arg, _METATABLE);
  }
return luaD_toclass(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  GObjectClass* klass =
  luaD_checkclass(L, 1);

  lua_pushfstring
  (L,
   "%sClass (%p)",
   g_type_name_from_class((GTypeClass*) klass),
   (guintptr) klass);
return 1;
}

static int
__index(lua_State* L)
{
  GObjectClass* klass =
  luaD_checkclass(L, 1);
  const gchar* t;

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    if G_LIKELY
      (g_type_is_a
       (G_TYPE_FROM_CLASS(klass),
        DS_TYPE_CALLABLE))
    {
      DsCallableIface* iface =
      g_type_interface_peek(klass, DS_TYPE_CALLABLE);
      g_assert(iface != NULL);

      DsClosure* closure =
      ds_callable_iface_get_field(iface, t);
      if G_LIKELY
        (closure != NULL
         && closure->flags & DS_CLOSURE_CONSTRUCTOR)
      {
        luaD_pushclosure(L, closure);
        return 1;
      }
    }
  }
return 0;
}

static int
__gc(lua_State* L)
{
  luaD_checkclass(L, 1);

  GObjectClass** ptr =
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

gboolean
_ds_luaclass_init(lua_State  *L,
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
_ds_luaclass_fini(lua_State* L)
{
}
