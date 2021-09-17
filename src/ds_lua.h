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
#ifndef __DS_LUA_INCLUDED__
#define __DS_LUA_INCLUDED__ 1
#include <glib.h>

/**
 * DS_LUA_ERROR:
 *
 * Error domain for DEUX_LUA. Errors in this domain will be from the #DsLuaError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_LUA_ERROR (ds_lua_error_quark())

/**
 * DsLuaError:
 * @DS_LUA_ERROR_FAILED: generic error condition.
 * @DS_LUA_ERROR_RUNTIME: runtime error.
 * @DS_LUA_ERROR_SYNTAX: syntax error.
 * @DS_LUA_ERROR_MEMORY: error allocating memory.
 * @DS_LUA_ERROR_RECURSIVE: recursive error at error handler.
 *
 * Error code returned by various ds_* functions.
 * Note that %DEUX_LUA_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_LUA_ERROR_FAILED,
  DS_LUA_ERROR_RUNTIME,
  DS_LUA_ERROR_SYNTAX,
  DS_LUA_ERROR_MEMORY,
  DS_LUA_ERROR_RECURSIVE,
} DsLuaError;

typedef struct _DsLualib DsLualib;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
ds_lua_error_quark();

/* Lua */
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct _DsLualib
{
  const gchar* name;
  lua_CFunction func;
};

gboolean
_ds_lua_init(lua_State  *L,
             GError    **error);
void
_ds_lua_fini(lua_State  *L);
gboolean
_ds_lualib_init(lua_State  *L,
                GError    **error);
void
_ds_lualib_fini(lua_State  *L);

gboolean
ds_xpcall(lua_State  *L,
          int         argc,
          int         retn,
          GError    **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUA_INCLUDED__
