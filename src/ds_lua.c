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
#include <ds_lua.h>

G_DEFINE_QUARK(ds-lua-error-quark,
               ds_lua_error);

static int
reporter(lua_State* L)
{
  const gchar* err = lua_tostring(L, 1);
  if G_UNLIKELY(err == NULL)
  {
    if(luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
      return 1;
    else
      err = lua_pushfstring(L, "(error object is a %s value)", lua_typename(L, 1));
  }

  luaL_traceback(L, L, err, 1);
return 1;
}

gboolean
ds_xpcall(lua_State  *L,
          int         argc,
          int         retn,
          GError    **error)
{
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  lua_pushcfunction(L, reporter);
  lua_insert(L, 1);

  int return_ =
  lua_pcall(L, argc, retn, 1);
  lua_remove(L, 1);

  switch(return_)
  {
  case LUA_OK:
  case LUA_YIELD:
    /* success! */
    break;
  case LUA_ERRMEM:
    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_MEMORY,
     "out of memory\r\n");
    break;
  case LUA_ERRERR:
    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_RECURSIVE,
     "recursive error\r\n");
    break;
  default:
    {
      const gchar* err =
      lua_tostring(L, -1);

      if G_UNLIKELY(err == NULL)
        err = "unknown error";

      gint code = 0;
      switch(return_)
      {
      case LUA_ERRRUN:
        code = DS_LUA_ERROR_RUNTIME;
        break;
      case LUA_ERRSYNTAX:
        code = DS_LUA_ERROR_SYNTAX;
        break;
      default:
        code = DS_LUA_ERROR_FAILED;
        break;
      }

      g_set_error
      (error,
       DS_LUA_ERROR,
       code,
       err);
    }
    break;
  }
}

/*
 * Lua
 * init / fini
 *
 */

gboolean
_ds_lua_init(lua_State  *L,
             GError    **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  luaL_openlibs(L);
  lua_settop(L, 0);

_error_:
return success;
}

gboolean
_ds_lua_fini(lua_State  *L,
             GError    **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

_error_:
return success;
}
