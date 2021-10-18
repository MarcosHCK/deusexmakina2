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
#ifndef __LUAD_CORE_INCLUDED__
#define __LUAD_CORE_INCLUDED__ 1
#include <ds_export.h>
#include <glib.h>

/**
 * LUAD_ERROR:
 *
 * Error domain for luaD. Errors in this domain will be from the #LuaDError enumeration.
 * See #GError for more information on error domains.
 */
#define LUAD_ERROR (luaD_error_quark())

/**
 * LuaDError:
 * @LUAD_ERROR_FAILED: generic error condition.
 * @LUAD_ERROR_RUNTIME: runtime error.
 * @LUAD_ERROR_SYNTAX: syntax error.
 * @LUAD_ERROR_MEMORY: error allocating memory.
 * @LUAD_ERROR_RECURSIVE: recursive error at error handler.
 *
 * Error code returned by luaD API functions.
 * Note that %LUAD_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum
{
  LUAD_ERROR_FAILED,
  LUAD_ERROR_RUNTIME,
  LUAD_ERROR_SYNTAX,
  LUAD_ERROR_MEMORY,
  LUAD_ERROR_RECURSIVE,
} LuaDError;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GQuark
luaD_error_quark();

/* Lua */
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

DEUSEXMAKINA2_API
lua_State*
luaD_new(GError** error);
DEUSEXMAKINA2_API
void
luaD_close(lua_State* L);
DEUSEXMAKINA2_API
gboolean
luaD_xpcall(lua_State  *L,
            int         argc,
            int         retn,
            GError    **error);


#if __LUAD_INSIDE__ == 1

G_GNUC_INTERNAL
G_GNUC_NORETURN
void
_luaD_throw_lerror(lua_State* L);
G_GNUC_INTERNAL
G_GNUC_NORETURN
void
_luaD_throw_gerror(lua_State* L, GError* error);

G_GNUC_INTERNAL
G_GNUC_NORETURN
void
_luaD_typeerror(lua_State* L, int arg, const gchar* expected);

#endif // __LUAD_INSIDE__

#if __cplusplus
}
#endif // __cplusplus

#endif // __LUAD_CORE_INCLUDED__
