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
#include <ds_luaclass.h>
#include <ds_luaclosure.h>
#include <ds_luaenum.h>
#include <ds_luaobj.h>
#include <ds_macros.h>
#include <gio/gio.h>

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
    g_type_from_name(typename_);
    if(g_type != G_TYPE_INVALID)
    {
      if(G_TYPE_IS_ENUM(g_type))
      {
        luaD_pushenum(L, g_type);
        cachetype(L, typename_);
        return 1;
      } else
      if(G_TYPE_IS_CLASSED(g_type))
      {
        luaD_pushclass(L, g_type);
        cachetype(L, typename_);
        return 1;
      } else
      {
        lua_pushfstring
        (L,
         "%s is an unclassed type\r\n",
         typename_);
        lua_error(L);
      }
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

  success =
  _ds_luaboxed_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _ds_luaclass_init(L, &tmp_err);
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
  _ds_luaenum_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  success =
  _ds_luaobj_init(L, &tmp_err);
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
#define set_macro(MacroName) \
  G_STMT_START { \
    lua_pushliteral(L, #MacroName ); \
    lua_pushliteral(L,  MacroName ); \
    lua_settable(L, -3); \
  } G_STMT_END

  set_macro(SCHEMASDIR);
  set_macro(GFXDIR);
  set_macro(PKGDATADIR);
  set_macro(PKGLIBEXECDIR);
  set_macro(ABSTOPBUILDDIR);

#undef set_macro

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
  _ds_luaobj_fini(L);
  _ds_luaenum_fini(L);
  _ds_luaclosure_fini(L);
  _ds_luaclass_fini(L);
  _ds_luaboxed_fini(L);
}
