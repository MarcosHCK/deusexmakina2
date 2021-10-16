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
#include <ds_luagtype_private.h>
#include <ds_macros.h>

#include <ds_callable.h>

#define _METATABLE "GType"

/*
 * Lua C-API
 *
 */

G_GNUC_NORETURN
static void
_ds_lua_throw_gerror(lua_State* L, GError* error)
{
  lua_pushfstring
  (L,
   "(%s: %i): %s: %i: %s\r\n",
   G_STRFUNC,
   __LINE__,
   g_quark_to_string(error->domain),
   error->code,
   error->message);
  g_error_free(error);
  lua_error(L);
  for(;;);
}

static void
dummy_notify(gpointer thing)
{
}

DsGType*
_luaD_pushgtype(lua_State* L, GType g_type, gpointer thing)
{
  DsGType* ptr = (DsGType*)
  lua_newuserdata(L, sizeof(DsGType));
  luaL_setmetatable(L, _METATABLE);

  ptr->g_type = g_type;
  ptr->thing = thing;
  ptr->notify = dummy_notify;

  ptr->fundamental = G_TYPE_IS_FUNDAMENTAL(g_type);
  ptr->classed = G_TYPE_IS_CLASSED(g_type);
  ptr->instantiable = G_TYPE_IS_INSTANTIATABLE(g_type);
  ptr->instanced = FALSE;
return ptr;
}

gboolean
_luaD_isgtype(lua_State* L, int idx)
{
  DsGType* ptr =
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

DsGType*
_luaD_togtype(lua_State* L, int idx)
{
  DsGType* ptr =
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

DsGType*
_luaD_checkgtype(lua_State* L, int arg)
{
  gboolean is =
  _luaD_isgtype(L, arg);
  if G_UNLIKELY(is == FALSE)
    _ds_lua_typeerror(L, arg, _METATABLE);
return _luaD_togtype(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  DsGType* ptr =
  _luaD_checkgtype(L, 1);

  lua_pushstring(L, g_type_name(ptr->g_type));
  if(ptr->instanced == TRUE)
  {
    lua_pushfstring(L, " (%p)", ptr->thing);
    lua_concat(L, 2);
  }
return 1;
}

static gboolean
__newindex_p(lua_State* L, DsGType* ptr, const gchar* t, GError** error)
{
  lua_pushnil(L);
return TRUE;
}

static int
__newindex(lua_State* L)
{
  const gchar* t = NULL;
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  DsGType* ptr = NULL;

  ptr =
  _luaD_checkgtype(L, 1);
  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    success =
    __newindex_p(L, ptr, t, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
      _ds_lua_throw_gerror(L, tmp_err);
    return (success == TRUE) ? 1 : 0;
  }
return 0;
}

static gboolean
__index_p(lua_State* L, DsGType* ptr, const gchar* t, GError** error)
{
  gboolean success = FALSE;
  GError* tmp_err = NULL;
  DsClosure* closure = NULL;
  DsGirHub* girhub = NULL;
  GValue value = G_VALUE_INIT;

  gboolean is_object = luaD_isobject(L, 1);
  gboolean is_class = luaD_isclass(L, 1);

  if(is_object == TRUE)
  {
    GObject* object = (GObject*) ptr->instance;

  /*
   * Check properties
   *
   */

    GParamSpec* pspec =
    g_object_class_find_property(G_OBJECT_GET_CLASS(object), t);
    if(pspec != NULL)
    {
      g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
      g_object_get_property(object, t, &value);

      success =
      _ds_pushvalue(L, &value, &tmp_err);
      g_value_unset(&value);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
      else
      {
        goto_success();
      }
    }
  }

  if(is_object == TRUE
     || is_class == TRUE)
  {
  /*
   * Check if object implements
   * 'DsCallable' interface (since
   * it is faster to use than GIR
   * repositories)
   *
   */

    if(g_type_is_a(ptr->g_type, DS_TYPE_CALLABLE))
    {
      DsCallableIface* iface = NULL;
      if(is_object == FALSE)
        iface = (DsCallableIface*)
        g_type_interface_peek(ptr->klass, DS_TYPE_CALLABLE);
      else
        iface =
        DS_CALLABLE_GET_IFACE(ptr->instance);

      closure =
      ds_callable_iface_get_field(iface, t);
      if(closure != NULL)
      {
        luaD_pushclosure(L, closure);
        ds_closure_unref(closure);
        goto_success();
      }
    }
  }

/*
 * Check using GIR repositories
 *
 */

  girhub =
  ds_gir_hub_get_default();

  closure =
  ds_gir_hub_get_method(girhub, ptr->g_type, t, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    g_value_unset(&value);
    goto_error();
  }
  else
  {
    if(closure != NULL)
    {
      luaD_pushclosure(L, closure);
      ds_closure_unref(closure);
      goto_success();
    }
    else
    {
      success =
      ds_gir_hub_get_field(girhub, ptr->g_type, t, &value, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
      else
      {
        if(G_IS_VALUE(&value))
        {
          success =
          _ds_pushvalue(L, &value, &tmp_err);
          if G_UNLIKELY(tmp_err != NULL)
          {
            g_propagate_error(error, tmp_err);
            goto_error();
          }
          else
          {
            goto_success();
          }
        }
        else
        {
          lua_pushnil(L);
          goto_success();
        }
      }
    }
  }

_error_:
return success;
}

static int
__index(lua_State* L)
{
  const gchar* t = NULL;
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  DsGType* ptr = NULL;

  ptr =
  _luaD_checkgtype(L, 1);
  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    success =
    __index_p(L, ptr, t, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
      _ds_lua_throw_gerror(L, tmp_err);
    return (success == TRUE) ? 1 : 0;
  }
return 0;
}

static GCancellable*
check_cancellable(lua_State* L, int idx, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GValue value = G_VALUE_INIT;
  GCancellable* cancellable = NULL;

  success =
  _ds_tovalue(L, idx, &value, G_TYPE_CANCELLABLE, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    if(g_error_matches
       (tmp_err,
        DS_VALUE_ERROR,
        DS_VALUE_ERROR_UNEXPECTED_TYPE) == TRUE)
    {
      return NULL;
    }
    else
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }
  else
  {
    cancellable =
    g_value_dup_object(&value);
  }

_error_:
  G_IS_VALUE(&value) ? g_value_unset(&value) : NULL;
  if G_UNLIKELY(success == FALSE)
    _g_object_unref0(cancellable);
return cancellable;
}

static gboolean
__call_p(lua_State* L, DsGType* ptr, const gchar* t, GError** error)
{
  GError* tmp_err = NULL;
  gboolean success = FALSE;
  int n_args = lua_gettop(L) - 1;
  GCancellable* cancellable = NULL;

  if(luaD_isclass(L, 1) == TRUE)
  {
    lua_pushliteral(L, "new");
    lua_gettable(L, 1);

    if(lua_isnil(L, -1) == FALSE)
    {
      lua_remove(L, 1);
      lua_insert(L, 1);
      lua_call(L, n_args, 1);
      goto_success();
    }
    else
    {
      GError* tmp_err = NULL;
      GObject* object = NULL;

    /*
     * Get cancellable
     *
     */

      cancellable =
      check_cancellable(L, n_args, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
      else
      {
        if(cancellable != NULL)
          --n_args;
      }

    /*
     * Create object
     *
     */

      object =
      _ds_lua_object_new(L, ptr->g_type, n_args >> 1, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }

    /*
     * If object is initable initialize it
     *
     */

      if(G_TYPE_IS_INITABLE(ptr->g_type))
      {
        g_initable_init(G_INITABLE(object), cancellable, &tmp_err);
        if G_UNLIKELY(tmp_err != NULL)
        {
          g_propagate_error(error, tmp_err);
          goto_error();
        }
      }

    /*
     * Push object
     *
     */

      if G_LIKELY(object != NULL)
      {
        luaD_pushobject(L, object);
        _g_object_unref0(object);
        goto_success();
      }
      else
      {
        lua_pushnil(L);
        goto_success();
      }
    }
  }

  lua_pushliteral(L, "attempt to call a ");
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, 1);
  lua_call(L, 1, 1);
  lua_pushliteral(L, " value");
  lua_concat(L, 3);
  lua_error(L);

_error_:
  _g_object_unref0(cancellable);
return success;
}

static int
__call(lua_State* L)
{
  const gchar* t = NULL;
  GError* tmp_err = NULL;
  gboolean success = TRUE;
  DsGType* ptr = NULL;

  ptr =
  _luaD_checkgtype(L, 1);

  success =
  __call_p(L, ptr, t, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
    _ds_lua_throw_gerror(L, tmp_err);
return (success == TRUE) ? 1 : 0;
}

static int
__gc(lua_State* L)
{
  DsGType* ptr =
  _luaD_checkgtype(L, 1);
  g_clear_pointer(&(ptr->thing), ptr->notify);
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__newindex", __newindex},
  {"__index", __index},
  {"__call", __call},
  {"__gc", __gc},
};

/*
 * Module
 *
 */

gboolean
_ds_luagtype_init(lua_State  *L,
                  GError    **error)
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
_ds_luagtype_fini(lua_State* L)
{
}
