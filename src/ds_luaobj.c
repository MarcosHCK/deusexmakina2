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
#include <ds_callable.h>
#include <ds_luaclosure.h>
#include <ds_luaobj.h>
#include <ds_macros.h>

#define _METATABLE "GObject"

/* defined in ds_utils.vala */
gboolean
_ds_tovalue(lua_State* L,
            gint idx,
            GValue* value,
            GType g_type,
            GError** error);
gboolean
_ds_pushvalue(lua_State* L,
              GValue* value,
              GError** error);

/*
 * Methods
 *
 */

void
luaD_tovalue(lua_State* L,
             gint idx,
             GValue* value,
             GType g_type)
{
  GError* tmp_err = NULL;
  _ds_tovalue(L, idx, value, g_type, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    lua_pushstring(L, tmp_err->message);
    g_error_free(tmp_err);
    lua_error(L);
  }
}

void
luaD_pushobject(lua_State  *L,
                GObject    *obj)
{
  GObject** ptr = (GObject**)
  lua_newuserdata(L, sizeof(GObject*));
  luaL_setmetatable(L, _METATABLE);
  (*ptr) = _g_object_ref0(obj);
}

gboolean
luaD_isobject(lua_State  *L,
              int         idx)
{
  GObject **ptr =
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

GObject*
luaD_toobject(lua_State  *L,
              int         idx)
{
  GObject **ptr =
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
        return (*ptr);
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

GObject*
luaD_checkobject(lua_State  *L,
                 int         arg)
{
  gboolean is =
  luaD_isobject(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _ds_lua_typeerror(L, arg, _METATABLE);
  }
return luaD_toobject(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  GObject* obj =
  luaD_checkobject(L, 1);

  lua_pushfstring
  (L,
   "%s (%p)",
   g_type_name_from_instance((GTypeInstance*)obj),
   (guintptr) obj);
return 1;
}

static int
__newindex(lua_State* L)
{
  const gchar* t;
  GObject* obj =
  luaD_checkobject(L, 1);

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    GObjectClass* klass =
    G_OBJECT_GET_CLASS(obj);

    GParamSpec* pspec =
    g_object_class_find_property(klass, t);
    if G_UNLIKELY(pspec != NULL)
    {
      GValue value = G_VALUE_INIT;
      GError* tmp_err = NULL;

      _ds_tovalue(L, 3, &value, G_PARAM_SPEC_VALUE_TYPE(pspec), &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_value_unset(&value);

        lua_pushfstring
        (L,
         "(%s: %i): %s: %i: %s\r\n",
         G_STRFUNC,
         __LINE__,
         g_quark_to_string(tmp_err->domain),
         tmp_err->code,
         tmp_err->message);
        lua_error(L);
        return 0;
      }
      else
      {
        g_object_setv(obj, 1, &t, &value);
        g_value_unset(&value);
        lua_pushvalue(L, 3);
        return 1;
      }
    }
    else
    {
      lua_pushfstring
      (L,
       "Unknown property '%s'\r\n",
       t);
      lua_error(L);
      return 0;
    }
  }
return 0;
}

static int
__index(lua_State* L)
{
  const gchar* t;
  GObject* obj =
  luaD_checkobject(L, 1);

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    if(DS_IS_CALLABLE(obj))
    {
      DsCallableIface* iface =
      DS_CALLABLE_GET_IFACE(obj);

      DsClosure* closure =
      ds_callable_iface_get_field(iface, t);
      if(closure != NULL)
      {
        luaD_pushclosure(L, closure);
        return 1;
      }
    }

    GObjectClass* klass =
    G_OBJECT_GET_CLASS(obj);

    GParamSpec* pspec =
    g_object_class_find_property(klass, t);
    if G_UNLIKELY(pspec != NULL)
    {
      GValue value = G_VALUE_INIT;
      GError* tmp_err = NULL;

      g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
      g_object_getv(obj, 1, &t, &value);
      _ds_pushvalue(L, &value, &tmp_err);
      g_value_unset(&value);

      if G_UNLIKELY(tmp_err != NULL)
      {
        lua_pushfstring
        (L,
         "(%s: %i): %s: %i: %s\r\n",
         G_STRFUNC,
         __LINE__,
         g_quark_to_string(tmp_err->domain),
         tmp_err->code,
         tmp_err->message);
        lua_error(L);
        return 0;
      }
      else
      {
        return 1;
      }
    }
  }
return 0;
}

static int
__gc(lua_State* L)
{
  luaD_checkobject(L, 1);

  GObject** ptr =
  lua_touserdata(L, 1);
  g_clear_object(ptr);
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__newindex", __newindex},
  {"__index", __index},
  {"__gc", __gc},
  {NULL, NULL},
};

gboolean
_ds_luaobj_init(lua_State  *L,
                GError    **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  luaL_newmetatable(L, _METATABLE);
  luaL_setfuncs(L, instance_mt, 0);
  lua_pushliteral(L, "__name");
  lua_pushstring(L, _METATABLE);
  lua_settable(L, -3);
  lua_pop(L, 1);

_error_:
return success;
}

void
_ds_luaobj_fini(lua_State* L)
{
}
