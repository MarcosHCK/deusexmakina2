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
#include <gio/gio.h>
#include <luad_lib.h>

/*
 * Library
 *
 */

static const
luaL_Reg lualib[] =
{
  {NULL, NULL},
};

/*
 * Module
 *
 */

gboolean
_luaD_lib_init(lua_State* L, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  GFile* current = NULL;
  GFile* child = NULL;

  /* keep this sync with ds_application.c */
  current = g_file_new_for_path(".");

/*
 * Library
 *
 */

#if LUA_VERSION_NUM >= 502
  luaL_newlib(L, lualib);
#else // LUA_VERSION_NUM
  lua_newtable(L);
  luaL_register(L, NULL, lualib);
#endif // LUA_VERSION_NUM

  /* module info */
  lua_pushliteral(L, "_COPYRIGHT");
  lua_pushliteral(L, "Copyright 2021-2022 MarcosHCK");
  lua_settable(L, -3);
  lua_pushliteral(L, "_VERSION");
  lua_pushliteral(L, PACKAGE_STRING);
  lua_settable(L, -3);

  /* configuration */
#if DEVELOPER == 1
# define set_macro(MacroName, __alternative__) \
    G_STMT_START { \
      lua_pushliteral(L, #MacroName ); \
      g_set_object(&child, g_file_get_child(current, __alternative__ )); \
      lua_pushstring(L, g_file_peek_path(child)); \
      lua_settable(L, -3); \
    } G_STMT_END
#else // DEVELOPER
# define set_macro(MacroName, __alternative__) \
    G_STMT_START { \
      lua_pushliteral(L, #MacroName ); \
      lua_pushliteral(L,  MacroName ); \
      lua_settable(L, -3); \
    } G_STMT_END
#endif // DEVELOPER
# define set_int_macro(MacroName) \
    G_STMT_START { \
      lua_pushliteral(L, #MacroName ); \
      lua_pushnumber(L,  MacroName ); \
      lua_settable(L, -3); \
    } G_STMT_END

  /* string macros */
  set_macro(    ASSETSDIR, "assets/");
  set_macro(       GFXDIR, "gfx/");
  set_macro(       GIRDIR, "gir/");
  set_macro(   SCHEMASDIR, "settings/");

  /* integer and boolean macros */
  set_int_macro(DEBUG);
  set_int_macro(DEVELOPER);

#undef set_macro

  /* set priority values */
  lua_pushliteral(L, "priority");
  lua_createtable(L, 5, 0);
  lua_pushnumber(L, G_MININT);
  lua_setfield(L, -2, "higher");
  lua_pushnumber(L, G_PRIORITY_HIGH);
  lua_setfield(L, -2, "high");
  lua_pushnumber(L, G_PRIORITY_DEFAULT);
  lua_setfield(L, -2, "default");
  lua_pushnumber(L, G_PRIORITY_LOW);
  lua_setfield(L, -2, "low");
  lua_pushnumber(L, G_MAXINT);
  lua_setfield(L, -2, "lower");
  lua_settable(L, -3);

  /* inject module onto package.loaded */
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_remove(L, -2);
  lua_pushliteral(L, "ds");
  lua_pushvalue(L, -3);
  lua_settable(L, -3);
  lua_pop(L, 2);

_error_:
  _g_object_unref0(current);
  _g_object_unref0(child);
return success;
}

void
_luaD_lib_fini(lua_State* L)
{
}
