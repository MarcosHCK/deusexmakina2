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
handle_script(lua_State* L, char** argx)
{
  int status;
  const char *fname = argx[0];

  status =
  luaL_loadfile(L, fname);
  if G_UNLIKELY(status == LUA_OK)
  {
    /* Fetch args from arg table. LUA_INIT or -e might have changed them. */
    int narg = 0;
    lua_getglobal(L, "arg");

    if G_LIKELY
      (lua_istable(L, -1))
    {
      do
      {
        narg++;
        lua_rawgeti(L, -narg, narg);
      }
      while(!lua_isnil(L, -1));

      lua_pop(L, 1);
      lua_remove(L, -narg);
      narg--;
    }
    else
    {
      lua_pop(L, 1);
    }

    status =
    lua_xpcall(L, narg, 0);
  }
return 0;
}

static void
createargtable(lua_State* L, gchar** argv, gint argc, gint argf)
{
  int i;
  lua_createtable(L, argc - argf, argf);

  for(i = 0;
      i < argc;
      i++)
  {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - argf);
  }

  lua_setglobal(L, "arg");
}

static int
pmain(lua_State *L)
{
  gint argc = (gint) lua_tointeger(L, 1);
  gchar** argv = (gchar**) lua_touserdata(L, 2);
  const gint script_idx = 1;
  gint return_, i;

  if G_UNLIKELY(!(argc >= 2))
    luaL_error(L, "assert failed!: argc >= 2");

  lua_gc(L, LUA_GCSTOP, 0);
  luaL_openlibs(L);
  lua_gc(L, LUA_GCRESTART, -1);

  createargtable(L, argv, argc, script_idx);

  return_ =
  luaL_loadfile(L, argv[script_idx]);
  if G_UNLIKELY(return_ != LUA_OK)
    lua_error(L);
return handle_script(L, argv + script_idx);
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
  lua_xpcall(L, 2, 0);
return return_;
}
