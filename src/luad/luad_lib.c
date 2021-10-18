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
#include <gmodule.h>
#include <luad_closure.h>
#include <luad_girhub.h>
#include <luad_lib.h>
#include <luad_value.h>

typedef GType (*GTypeGetFunc) (void);

#define cachetype(L,t) \
  G_STMT_START { \
    /*  1 -> ds.type      */ \
    /* -1 -> class value  */ \
    lua_pushstring((L), (t)); \
    lua_pushvalue((L), -2); \
    lua_settable((L), 1); \
  } G_STMT_END

static gchar*
type_name_mangle(const gchar* name)
{
  GString* symbol_name = g_string_new("");
  gint i;

  for(i = 0;name[i] != '\0';i++)
  {
    /* skip if uppercase, first or previous is uppercase */
    if((name[i] == g_ascii_toupper(name[i])
        && i > 0
        && name[i-1] != g_ascii_toupper(name[i-1]))
       || (i > 2
           && name[i] == g_ascii_toupper(name[i])
           && name[i-1] == g_ascii_toupper(name[i-1])
           && name[i-2] == g_ascii_toupper(name[i-2])))
    {
      g_string_append_c(symbol_name, '_');
    }

    g_string_append_c(symbol_name, g_ascii_tolower(name[i]));
  }

  g_string_append(symbol_name, "_get_type");
return g_string_free(symbol_name, FALSE);
}

static GType
resolve_type_lazily(const gchar* name)
{
  static
  GModule* module = NULL;
  GTypeGetFunc func = NULL;
  GType g_type = G_TYPE_INVALID;
  gchar* symbol;

  if G_UNLIKELY(module == NULL)
    module = g_module_open(NULL, 0);

  symbol = type_name_mangle(name);

  if(g_module_symbol
     (module,
      symbol,
      (gpointer) &func))
  {
    g_type = func();
  }

  g_free(symbol);
return g_type;
}

static GType
resolve_type(const gchar* name)
{
  GType g_type;

  if((g_type = g_type_from_name(name)) != G_TYPE_INVALID)
    return g_type;
  if((g_type = resolve_type_lazily(name)) != G_TYPE_INVALID)
    return g_type;
return G_TYPE_INVALID;
}

/*
 * __index()! & __newindex()!
 *
 */

static int
__newindex(lua_State* L)
{
return 0;
}

static int
__index(lua_State* L)
{
  const gchar* t;
  GType g_type;

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    g_type =
    resolve_type(t);
    if G_LIKELY
      (g_type != G_TYPE_INVALID)
    {
      _luaD_pushlvalue(L, g_type, NULL);
      return 1;
    }
  }
return 0;
}

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

/*
 * Initialization
 *
 */

  success =
  _luaD_lclosure_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _luaD_girhub_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _luaD_lvalue_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

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

  /* types table */
  lua_pushliteral(L, "type");
  lua_createtable(L, 0, 0);
  lua_createtable(L, 0, 1);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, __newindex);
  lua_settable(L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, __index);
  lua_settable(L, -3);
  lua_setmetatable(L, -2);
  lua_settable(L, -3);

  /* configuration */
#if DEVELOPER == 1
# define set_macro(MacroName, __alternative__) \
    G_STMT_START { \
      lua_pushliteral(L, #MacroName ); \
      lua_pushliteral(L,  __alternative__ ); \
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
  set_macro(    ASSETSDIR, ABSTOPBUILDDIR "/assets");
  set_macro(       GFXDIR, ABSTOPBUILDDIR "/gfx");
  set_macro(   SCHEMASDIR, ABSTOPBUILDDIR "/settings");

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
return success;
}

void
_luaD_lib_fini(lua_State* L)
{
/*
 * Finalization
 *
 */

  _luaD_lvalue_fini(L);
  _luaD_girhub_fini(L);
  _luaD_lclosure_fini(L);
}
