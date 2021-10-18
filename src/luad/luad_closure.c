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
#include <luad_closure.h>

#define _METATABLE  "LClosure"
#define _THREAD     "LClosureThread"

LClosure*
_luaD_pushlclosure(lua_State* L)
{
return NULL;
}

gboolean
_luaD_islclosure(lua_State* L, int idx)
{
  LClosure* ptr =
  lua_touserdata(L, idx);

  if G_LIKELY(ptr != NULL)
  {
    if G_LIKELY
      (lua_getmetatable
       (L, idx) == TRUE)
    {
      luaL_getmetatable(L, _METATABLE);
      if G_LIKELY
        (lua_rawequal
         (L, -1, -2) == TRUE)
      {
        lua_pop(L, 2);
        return TRUE;
      }
      else
      {
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }
  }
return FALSE;
}

LClosure*
_luaD_tolclosure(lua_State* L, int idx)
{
  LClosure* ptr =
  lua_touserdata(L, idx);

  if G_LIKELY(ptr != NULL)
  {
    if G_LIKELY
      (lua_getmetatable
       (L, idx) == TRUE)
    {
      luaL_getmetatable(L, _METATABLE);
      if G_LIKELY
        (lua_rawequal
         (L, -1, -2) == TRUE)
      {
        lua_pop(L, 2);
        return ptr;
      }
      else
      {
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }
  }
return NULL;
}

LClosure*
_luaD_checklclosure(lua_State* L, int arg)
{
  gboolean is =
  _luaD_islclosure(L, arg);
  if G_UNLIKELY(is == FALSE)
    _luaD_typeerror(L, arg, _METATABLE);
return _luaD_tolclosure(L, arg);
}

/*
 * Metatable
 *
 */

static const
luaL_Reg instance_mt[] =
{
  {NULL, NULL},
};

/*
 * Module
 *
 */

gboolean
_luaD_lclosure_init(lua_State* L, GError** error)
{
  luaL_newmetatable(L, _METATABLE);
  luaL_setfuncs(L, instance_mt, 0);
  lua_pushliteral(L, "__name");
  lua_pushliteral(L, _METATABLE);
  lua_settable(L, -3);
  lua_pop(L, 1);
return TRUE;
}

void
_luaD_lclosure_fini(lua_State* L)
{
}
