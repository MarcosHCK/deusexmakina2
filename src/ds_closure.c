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
#include <ds_closure.h>
#include <ds_macros.h>
#include <ds_luaclosure.h>
#include <gio/gio.h>

#define _METATABLE "DsClosure"

G_STATIC_ASSERT(G_STRUCT_OFFSET(GCClosure, callback) == G_STRUCT_OFFSET(DsClosure, callback));

/* defined in ds_utils.vala */
gboolean
_ds_pushvalue(lua_State* L,
              GValue* value,
              GError** error);
gboolean
_ds_tovalue(lua_State* L,
            gint idx,
            GValue* value,
            GType g_type,
            GError** error);

/*
 * Methods
 *
 */

void
luaD_pushclosure(lua_State *L,
                 DsClosure *closure)
{
  DsClosure** ptr = (DsClosure**)
  lua_newuserdata(L, sizeof(DsClosure*));
  luaL_setmetatable(L, _METATABLE);

  g_closure_ref((GClosure*) closure);
  g_closure_sink((GClosure*) closure);
  (*ptr) = closure;
}

/* Taken as-in from LuaJIT code */
static int
_typeerror(lua_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  if (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  else if (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  else
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  return luaL_argerror(L, arg, msg);
}

gboolean
luaD_isclosure(lua_State  *L,
               int         idx)
{
  DsClosure **ptr =
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

DsClosure*
luaD_toclosure(lua_State  *L,
               int         idx)
{
  DsClosure **ptr =
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

DsClosure*
luaD_checkclosure(lua_State  *L,
                  int         arg)
{
  gboolean is =
  luaD_isclosure(L, arg);
  if G_UNLIKELY(is == FALSE)
  {
    _typeerror(L, arg, _METATABLE);
  }
return luaD_toclosure(L, arg);
}

/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  DsClosure* closure =
  luaD_checkclosure(L, 1);

  lua_pushfstring
  (L,
   "%s (%p)",
   _METATABLE,
   (guintptr) closure);
return 1;
}

G_STATIC_ASSERT(FALSE == 0);
G_STATIC_ASSERT(TRUE == 1);

static int
_ds_closure__call(lua_State* L,
                  DsClosure* closure,
                  GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  gboolean constructor = closure->flags & DS_CLOSURE_CONSTRUCTOR;
  guint i, j, n_params = closure->n_params;

  GValue* params = NULL;
  GValue return_ = G_VALUE_INIT;

  GType rtype = closure->return_type & ~DS_INVOKE_STATIC_SCOPE;
  gboolean static_scope = closure->return_type & DS_INVOKE_STATIC_SCOPE;
  if(closure->return_type != G_TYPE_NONE)
  {
    g_value_init(&return_, rtype);
  }

  GValue* params_ = g_alloca(sizeof(GValue) * (n_params + 1));
  memset(params_, 0, sizeof(GValue) * (n_params + 1));
  params = params_ + 1;

  g_value_init(params_, G_TYPE_OBJECT);
  if(constructor == FALSE)
  {
    gboolean is =
    luaD_isobject(L, 2);
    if(is == FALSE)
    {
      g_set_error
      (error,
       G_IO_ERROR,
       G_IO_ERROR_FAILED,
       "First argument should be an object instance\r\n");
      goto_error();
    }
    else
    {
      GObject* object = luaD_toobject(L, 2);
      g_value_set_object(params_, object);
    }
  }

  for(i = 0, j = (constructor) ? 2 : 3;
      i < closure->n_params;
      i++)
  {
    GType ptype = closure->params[i] & ~DS_INVOKE_STATIC_SCOPE;
    _ds_tovalue(L, j++, &(params[i]), ptype, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

  g_closure_invoke
  ((GClosure*)
   closure,
   &return_,
   n_params + 1,
   params_,
   NULL);

  if(rtype != G_TYPE_NONE)
  {
    _ds_pushvalue(L, &return_, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

_error_:
  g_value_unset(&return_);
  for(i = 0;
      i < n_params;
      i++)
  {
    g_value_unset(&(params[i]));
  }
  if(closure->flags & DS_CLOSURE_CONSTRUCTOR)
  {
    g_value_unset(params - 1);
  }
return (int) success;
}

static int
__call(lua_State* L)
{
  DsClosure* closure =
  luaD_checkclosure(L, 1);
  GError* tmp_err = NULL;
  guint return_ = 0;

  return_ =
  _ds_closure__call(L, closure, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    lua_pushstring(L, tmp_err->message);
    g_error_free(tmp_err);
    lua_error(L);
    return 0;
  }
return return_;
}

static int
__gc(lua_State* L)
{
  luaD_checkclosure(L, 1);

  DsClosure** ptr =
  lua_touserdata(L, 1);
  g_closure_unref
  ((GClosure*)
   *ptr);
return 0;
}

static const
luaL_Reg instance_mt[] =
{
  {"__tostring", __tostring},
  {"__call", __call},
  {"__gc", __gc},
  {NULL, NULL},
};

gboolean
_ds_luaclosure_init(lua_State  *L,
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
_ds_luaclosure_fini(lua_State* L)
{
}
