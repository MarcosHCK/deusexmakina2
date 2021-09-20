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
#include <ds_luaboxed.h>

typedef struct _DsBoxed DsBoxed;

#define _METATABLE "GBoxed"

struct _DsBoxed
{
  GType type;
  gpointer ptr;
};

/*
 * Methods
 *
 */

void
luaD_pushboxed(lua_State *L,
               GType      g_type,
               gpointer   ptr)
{
  DsBoxed* boxed = (DsBoxed*)
  lua_newuserdata(L, sizeof(DsBoxed));
  luaL_setmetatable(L, _METATABLE);

  boxed->type = g_type;
  boxed->ptr = g_boxed_copy(g_type, ptr);
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
luaD_isboxed(lua_State  *L,
             int         idx)
{
  DsBoxed* boxed =
  lua_touserdata(L, idx);

  if G_LIKELY(boxed != NULL)
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

gpointer
luaD_toboxed(lua_State  *L,
             int         idx,
             GType      *g_type)
{
  DsBoxed* boxed =
  lua_touserdata(L, idx);

  if G_LIKELY(boxed != NULL)
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
        if(g_type != NULL)
          *g_type = boxed->type;
        return boxed->ptr;
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

gpointer
luaD_checkboxed(lua_State *L,
                int        arg,
                GType     *g_type)
{
  gboolean is =
  luaD_isboxed(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _typeerror(L, arg, _METATABLE);
  }
return luaD_toboxed(L, arg, g_type);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  GType btype = G_TYPE_INVALID;
  gpointer boxed =
  luaD_checkboxed(L, 1, &btype);

  lua_pushfstring
  (L,
   "%s (%p)",
   g_type_name(btype),
   (guintptr) boxed);
return 1;
}

static int
__index(lua_State* L)
{
  GType btype = G_TYPE_INVALID;
  gpointer boxed =
  luaD_checkboxed(L, 1, &btype);

  if(g_type_is_a(btype, G_TYPE_ERROR))
  {
    const gchar* t =
    luaL_checkstring(L, 2);
    if(!strcmp(t, "ref"))
    {
      DsBoxed* boxed =
      lua_touserdata(L, 1);
      lua_pushlightuserdata(L, &(boxed->ptr));
      return 1;
    }
  }
return 0;
}

static int
__gc(lua_State* L)
{
  GType btype = G_TYPE_INVALID;
  gpointer boxed =
  luaD_checkboxed(L, 1, &btype);
  g_boxed_free(btype, boxed);
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
_ds_luaboxed_init(lua_State  *L,
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
_ds_luaboxed_fini(lua_State* L)
{
}
