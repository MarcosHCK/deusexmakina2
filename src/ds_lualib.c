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
#include <ds_luaclosure.h>
#include <ds_luagir.h>
#include <ds_luagtype.h>
#include <ds_luaqdata.h>
#include <ds_macros.h>
#include <gio/gio.h>

typedef GType (*GTypeGetFunc) (void);

/* Some kind of static assert */
#include <luajit.h>

/*
 * Functions table
 *
 */

static const
luaL_Reg lualib[] =
{
  {NULL, NULL},
};

/*
 * Type table metaindexes
 *
 */

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
  GTypeGetFunc func;
  GType gtype = G_TYPE_INVALID;
  gchar* symbol;

  if G_UNLIKELY(module == NULL)
  {
    module = g_module_open(NULL, 0);
  }

  symbol = type_name_mangle(name);

  if(g_module_symbol
     (module,
      symbol,
      (gpointer)&func))
  {
    gtype = func();
  }

  g_free(symbol);
return gtype;
}

static GType
resolve_type(const gchar* name)
{
  GType gtype;

  if((gtype = g_type_from_name(name)) != G_TYPE_INVALID)
    return gtype;
  if((gtype = resolve_type_lazily(name)) != G_TYPE_INVALID)
    return gtype;
return G_TYPE_INVALID;
}

static int
__index(lua_State* L)
{
  const gchar* typename_;
  if G_LIKELY
    (lua_isstring(L, 2) == TRUE)
  {
    typename_ = lua_tostring(L, 2);
    if G_UNLIKELY(typename_ == NULL)
    {
      return 0;
    }

    GType g_type =
    resolve_type(typename_);
    if(g_type != G_TYPE_INVALID)
    {
      luaD_pushgtype(L, g_type);
      return 1;
/*
      if(G_TYPE_IS_ENUM(g_type))
      {
        luaD_pushenum(L, g_type);
        cachetype(L, typename_);
        return 1;
      } else
      {
        luaD_pushclass(L, g_type);
        cachetype(L, typename_);
        return 1;
      }
*/
    }
  }
return 0;
}

/*
 * luaopen_*
 *
 */

G_GNUC_INTERNAL
gboolean
_ds_lualib_init(lua_State  *L,
                GError    **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Dependencies
 * sorted in by dependency
 *
 */

  success =
  _ds_luaqdata_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _ds_luaclosure_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _ds_luagir_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Generic type
 * sorted alphabetically
 *
 */

  success =
  _ds_luagtype_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

#if LUA_VERSION_NUM >= 502
  luaL_newlib(L, lualib);
#else
  lua_newtable(L);
  luaL_register(L, NULL, lualib);
#endif

  /* module info */
  lua_pushliteral(L, "Copyright 2021-2022 MarcosHCK");
  lua_setfield(L, -2, "_COPYRIGHT");
  lua_pushliteral(L, PACKAGE_STRING);
  lua_setfield(L, -2, "_VERSION");

  /* create types table */
  lua_pushliteral(L, "type");
  lua_createtable(L, 0, 0);
  lua_createtable(L, 0, 1);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, __index);
  lua_settable(L, -3);
  lua_setmetatable(L, -2);
  lua_settable(L, -3);

  /* Set compile-time strings */
#define set_macro(MacroName, __debug__) \
  G_STMT_START { \
    lua_pushliteral(L, #MacroName ); \
    if G_UNLIKELY(debug == TRUE) \
      lua_pushliteral(L,  __debug__ ); \
    else \
      lua_pushliteral(L,  MacroName ); \
    lua_settable(L, -3); \
  } G_STMT_END

  gboolean debug = FALSE;
#if DEBUG
  debug = TRUE;
#else
  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
    debug = TRUE;
  }
#endif // DEBUG

  set_macro(    ASSETSDIR, ABSTOPBUILDDIR "/assets");
  set_macro(       GFXDIR, ABSTOPBUILDDIR "/gfx");
  set_macro(   SCHEMASDIR, ABSTOPBUILDDIR "/settings");

#undef set_macro

  /* set priority table */
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

G_GNUC_INTERNAL
void
_ds_lualib_fini(lua_State* L)
{

/*
 * Generic type
 *
 */

  _ds_luagtype_fini(L);

/*
 * Dependencies
 *
 */

  _ds_luagir_fini(L);
  _ds_luaclosure_fini(L);
  _ds_luaqdata_fini(L);
}
