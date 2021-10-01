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
#include <ds_luaclass.h>
#include <ds_luaclosure.h>
#include <ds_luagir.h>
#include <ds_macros.h>

#define _METATABLE "GType"
#define _g_type_class_unref0(var) ((var == NULL) ? NULL : (var = (g_type_class_unref (var), NULL)))

/*
 * Methods
 *
 */

void
luaD_pushclass(lua_State *L,
               GType      g_type)
{
  DsClass* ptr = (DsClass*)
  lua_newuserdata(L, sizeof(DsClass));
  luaL_setmetatable(L, _METATABLE);

  ptr->g_type = g_type;
  ptr->fundamental = G_TYPE_IS_FUNDAMENTAL(g_type);
  ptr->classed = G_TYPE_IS_CLASSED(g_type);
  ptr->instantiatable = G_TYPE_IS_INSTANTIATABLE(g_type);

  if(ptr->classed)
    ptr->klass = g_type_class_ref(g_type);
  else
    ptr->klass = NULL;
}

gboolean
luaD_isclass(lua_State  *L,
             int         idx)
{
  DsClass* ptr =
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

DsClass*
luaD_toclass(lua_State  *L,
             int         idx)
{
  DsClass* ptr =
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

DsClass*
luaD_checkclass(lua_State  *L,
                int         arg)
{
  gboolean is =
  luaD_isclass(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _ds_lua_typeerror(L, arg, _METATABLE);
  }
return luaD_toclass(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  DsClass* ptr =
  luaD_checkclass(L, 1);

  lua_pushfstring
  (L,
   "%s",
   g_type_name(ptr->g_type),
   (guintptr) ptr);
return 1;
}

static int
__index(lua_State* L)
{
  DsClass* ptr =
  luaD_checkclass(L, 1);
  const gchar* t;

  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    if G_LIKELY(g_type_is_a(ptr->g_type, DS_TYPE_CALLABLE))
    {
      DsCallableIface* iface =
      g_type_interface_peek(ptr->klass, DS_TYPE_CALLABLE);
      g_assert(iface != NULL);

      DsClosure* closure =
      ds_callable_iface_get_field(iface, t);
      if G_LIKELY
        (closure != NULL
         && closure->flags & DS_CLOSURE_CONSTRUCTOR)
      {
        luaD_pushclosure(L, closure);
        ds_closure_unref(closure);
        return 1;
      }
    }
    else
    {
      DsGirHub* hub = ds_gir_hub_get_default();
      DsClosure* closure = NULL;
      GError* tmp_err = NULL;

      closure =
      ds_gir_hub_get_method(hub, ptr->g_type, t, &tmp_err);
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
      if(closure != NULL)
      {
        luaD_pushclosure(L, closure);
        g_closure_unref((GClosure*) closure);
        return 1;
      }
    }
  }
return 0;
}

static int
__call(lua_State* L)
{
  DsClass* ptr =
  luaD_checkclass(L, 1);
  int n_args = lua_gettop(L) - 1;

  lua_pushliteral(L, "new");
  lua_gettable(L, 1);
  if(lua_isfunction(L, -1) == TRUE)
  {
    lua_remove(L, 1);
    lua_insert(L, 1);
    lua_call(L, n_args, 1);
    return 1;
  }
  else
  {
    GError* tmp_err = NULL;
    GObject* object = NULL;

    object =
    _ds_lua_object_new(L, ptr->g_type, n_args >> 1, &tmp_err);
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
      _g_object_unref0(object);
      lua_error(L);
    }

    luaD_pushobject(L, object);
    _g_object_unref0(object);
    return 1;
  }
return 0;
}

static int
__gc(lua_State* L)
{
  DsClass* ptr =
  luaD_checkclass(L, 1);
  _g_type_class_unref0(ptr->klass);
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__index", __index},
  {"__call", __call},
  {"__gc", __gc},
  {NULL, NULL},
};

G_GNUC_INTERNAL
gboolean
_ds_luaclass_init(lua_State  *L,
                GError    **error)
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
_ds_luaclass_fini(lua_State* L)
{
}
