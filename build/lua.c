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
#include <glib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static int
reporter(lua_State* L)
{
  const gchar* err = lua_tostring(L, 1);
  if G_UNLIKELY(err == NULL)
  {
    if(luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
      err = lua_tostring(L, -1);
    else
      err = lua_pushfstring(L, "(error object is a %s value)", lua_typename(L, 1));
  }

  luaL_traceback(L, L, err, 1);
return 1;
}

static int
lua_xpcall(lua_State *L, int n_args, int n_rets)
{
  gint return_;
  gint repo_idx = lua_gettop(L) - n_args;
  lua_pushcfunction(L, reporter);
  lua_insert(L, repo_idx);

  return_ =
  lua_pcall(L, n_args, n_rets, repo_idx);
  lua_remove(L, repo_idx);

  switch(return_)
  {
  case LUA_OK:
  case LUA_YIELD:
    /* success! */
    break;
  case LUA_ERRMEM:
    g_critical("out of memory\r\n");
    g_assert_not_reached();
    break;
  case LUA_ERRERR:
    g_critical("recursive error\r\n");
    g_assert_not_reached();
    break;
  default:
    {
      const gchar* err =
      lua_tostring(L, -1);

      if G_UNLIKELY(err == NULL)
        err = "unknown error";

      g_critical
      ("lua_pcall(): failed!: %s\r\n",
       err);
    }
    break;
  }
return return_;
}

static int
pmain(lua_State *L)
{
  gint argc = (int)lua_tointeger(L, 1);
  gchar** argv = (char**)lua_touserdata(L, 2);
  const gint script_idx = 1;
  gint return_, i, n;

  if G_UNLIKELY(!(argc >= 2))
    luaL_error(L, "assert failed!: argc >= 2");

  luaL_openlibs(L);

  return_ =
  luaL_loadfile(L, argv[script_idx]);
  if G_UNLIKELY(return_ != LUA_OK)
    lua_error(L);

  for(i = script_idx, n = 0;
      i < argc;
      i++, n++)
  {
    lua_pushstring(L, argv[i]);
  }

  lua_call(L, n, 0);
return 0;
}

/*
 * main()!
 *
 */

int
main(int argc, char* argv[])
{
  gint return_;
  lua_State* L;

  L = luaL_newstate();
  if G_UNLIKELY(L == NULL)
  {
    g_critical("luaL_newstate()!: failed!\r\n");
    g_assert_not_reached();
  }

  lua_pushcfunction(L, pmain);
  lua_pushinteger(L, argc);
  lua_pushlightuserdata(L, argv);

  return_ =
  lua_xpcall(L, 2, 1);
return return_;
}
