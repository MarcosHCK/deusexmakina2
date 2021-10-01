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
#include <ds_luaqdata.h>

struct _DsQData
{
  gpointer user_data;
  GDestroyNotify notify;
};

#define _METATABLE "DsQData"

typedef struct _DsQData DsQData;

/*
 * Methods
 *
 */

void
luaD_pushqdata(lua_State *L)
{
  DsQData* qdata = (DsQData*)
  lua_newuserdata(L, sizeof(DsQData));
  luaL_setmetatable(L, _METATABLE);
  qdata->user_data = NULL;
  qdata->notify = NULL;
}

gboolean
luaD_isqdata(lua_State *L,
             int        idx)
{
  DsQData* qdata =
  lua_touserdata(L, idx);

  if G_LIKELY(qdata != NULL)
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

DsQData*
luaD_toqdata(lua_State  *L,
            int         idx)
{
  DsQData* qdata =
  lua_touserdata(L, idx);

  if G_LIKELY(qdata != NULL)
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
        return qdata;
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

DsQData*
luaD_checkqdata(lua_State  *L,
                int         arg)
{
  gboolean is =
  luaD_isqdata(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _ds_lua_typeerror(L, arg, _METATABLE);
  }
return luaD_toqdata(L, arg);
}

/*
 * C API
 *
 */

gpointer
luaD_get_qdata(lua_State* L, GQuark quark)
{
  lua_getfield
  (L,
   LUA_REGISTRYINDEX,
   g_quark_to_string
   (quark));


  if(luaD_isqdata(L, -1) == FALSE)
  {
#if DEBUG == 1
    if(lua_isnil(L, -1) == FALSE)
    {
      g_critical
      ("(%s: %i) Invalid value '%s'\r\n",
       G_STRFUNC, __LINE__,
       g_quark_to_string(quark));
      g_assert_not_reached();
    }
#endif // DEBUG
    lua_pop(L, 1);
    return NULL;
  }

  DsQData* qdata =
  luaD_toqdata(L, -1);
  lua_pop(L, 1);
return qdata->user_data;
}

gpointer
luaD_steal_qdata(lua_State* L, GQuark quark)
{
  lua_getfield
  (L,
   LUA_REGISTRYINDEX,
   g_quark_to_string
   (quark));


  if(luaD_isqdata(L, -1) == FALSE)
  {
#if DEBUG == 1
    if(lua_isnil(L, -1) == FALSE)
    {
      g_critical
      ("(%s: %i) Invalid value '%s'\r\n",
       G_STRFUNC, __LINE__,
       g_quark_to_string(quark));
      g_assert_not_reached();
    }
#endif // DEBUG
    lua_pop(L, 1);
    return NULL;
  }

  DsQData* qdata =
  luaD_toqdata(L, -1);
  gpointer user_data =
  qdata->user_data;
  qdata->user_data = NULL;
  qdata->notify = NULL;
return user_data;
}

void
luaD_set_qdata_full(lua_State* L, GQuark quark, gpointer user_data, GDestroyNotify notify)
{
  lua_getfield
  (L,
   LUA_REGISTRYINDEX,
   g_quark_to_string(quark));

  if(luaD_isqdata(L, -1) == FALSE)
  {
#if DEBUG == 1
    if(lua_isnil(L, -1) == FALSE)
    {
      g_critical
      ("(%s: %i) Invalid value '%s'\r\n",
       G_STRFUNC, __LINE__,
       g_quark_to_string(quark));
      g_assert_not_reached();
    }
#endif // DEBUG
    lua_pop(L, 1);

    luaD_pushqdata(L);
    lua_pushvalue(L, -1);

    lua_setfield
    (L,
     LUA_REGISTRYINDEX,
     g_quark_to_string(quark));
  }

  DsQData* qdata =
  luaD_toqdata(L, -1);

  if(qdata->notify != NULL)
  {
    qdata->notify(qdata->user_data);
  }

  qdata->user_data = user_data;
  qdata->notify = notify;
}

void
luaD_set_qdata(lua_State* L, GQuark quark, gpointer user_data)
{
  luaD_set_qdata_full(L, quark, user_data, NULL);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  DsQData* qdata =
  luaD_checkqdata(L, 1);

  lua_pushfstring
  (L,
   "%s (%p)",
   _METATABLE,
   (guintptr) qdata);
return 1;
}

static int
__gc(lua_State* L)
{
  luaD_checkqdata(L, 1);
  DsQData* qdata =
  lua_touserdata(L, 1);

  if(qdata->notify != NULL)
  {
    qdata->notify(qdata->user_data);
    qdata->notify = NULL;
  }

  qdata->user_data = NULL;
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__gc", __gc},
  {NULL, NULL},
};

G_GNUC_INTERNAL
gboolean
_ds_luaqdata_init(lua_State *L,
                 GError   **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  luaL_newmetatable(L, _METATABLE);
  luaL_setfuncs(L, instance_mt, 0);
  lua_pushliteral(L, "__name");
  lua_pushliteral(L, _METATABLE);
  lua_settable(L, -3);
  lua_pop(L, 1);

_error_:
return success;
}

G_GNUC_INTERNAL
void
_ds_luaqdata_fini(lua_State* L)
{
}
