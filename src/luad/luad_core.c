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
#include <ds_macros.h>
#include <luad_core.h>
#include <luad_lib.h>

G_DEFINE_QUARK(luaD-error-quark,
               luaD_error);

#define _lua_close0(var) (var == NULL) ? NULL : (var = (lua_close (var), NULL))

/*
 * Methods
 *
 */

lua_State*
luaD_new(GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  lua_State* L = NULL;

  L =
  luaL_newstate();
  if G_UNLIKELY(L == NULL)
  {
    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_FAILED,
     "luaL_newstate(): failed!: unknown error\r\n");
    goto_error();
  }

  luaL_openlibs(L);

  success =
  _luaD_lib_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    _lua_close0(L);
  else
    lua_settop(L, 0);
return L;
}

void
luaD_close(lua_State* L)
{
  _luaD_lib_fini(L);
  lua_close(L);
}

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

gboolean
luaD_xpcall(lua_State  *L,
            int         argc,
            int         retn,
            GError    **error)
{
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  gboolean success = FALSE;

  lua_pushcfunction(L, reporter);
  lua_insert(L, 1);

  int return_ =
  lua_pcall(L, argc, retn, 1);
  lua_remove(L, 1);

  switch(return_)
  {
  case LUA_OK:
    G_GNUC_FALLTHROUGH;
  case LUA_YIELD:
    /* success! */
    success = TRUE;
    break;
  case LUA_ERRMEM:
    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_MEMORY,
     "(%s: %i): out of memory\r\n",
     G_STRFUNC,
     __LINE__);
    break;
  case LUA_ERRERR:
    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_RECURSIVE,
     "(%s: %i): recursive error\r\n",
     G_STRFUNC,
     __LINE__);
    break;
  case LUA_ERRRUN:
    G_GNUC_FALLTHROUGH;
  case LUA_ERRSYNTAX:
    {
      const gchar* err = lua_tostring(L, -1);
      if G_UNLIKELY(err == NULL)
        err = "unknown error";

      gint code =
      (return_ == LUA_ERRRUN)
      ? LUAD_ERROR_RUNTIME
      : LUAD_ERROR_SYNTAX;

      g_set_error_literal(error, LUAD_ERROR, code, err);
    }
    break;
  default:
    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_MEMORY,
     "(%s: %i): unknown error code %i\r\n",
     G_STRFUNC,
     __LINE__,
     return_);
    break;
  }

return success;
}

void
_luaD_throw_lerror(lua_State* L)
{
/*
 * What's the point with this function anyway?
 * Because 'lua_error', at least on LuaJIT version
 * I'm working right now, isn't declared as no-return.
 *
 */

  lua_error(L);
for(;;);
}

void
_luaD_throw_gerror(lua_State* L, GError* error)
{
  lua_pushfstring
  (L,
   "(%s: %i): %s: %i: %s\r\n",
   G_STRFUNC,
   __LINE__,
   g_quark_to_string(error->domain),
   error->code,
   error->message);
  g_error_free(error);
  _luaD_throw_lerror(L);
}

void
_luaD_typeerror(lua_State* L, int arg, const gchar* tname)
{
  /* Taken as-in from LuaJIT code */
  const char *msg;
  const char *typearg;

  if(luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);
  else
  if(lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";
  else
    typearg = luaL_typename(L, arg);

  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  luaL_argerror(L, arg, msg);
for(;;);
}
