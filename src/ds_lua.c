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
#include <ds_error.h>
#include <ds_lua.h>
#include <ds_macros.h>

G_DEFINE_QUARK(ds-lua-error-quark,
               ds_lua_error);

typedef struct _DsQData DsQData;

struct _DsQData
{
  gpointer user_data;
  GDestroyNotify notify;
};

/*
 * Methods
 *
 */

G_GNUC_INTERNAL
G_GNUC_NORETURN
int
_ds_lua_typeerror(lua_State  *L,
                  int         arg,
                  const char *tname)
{
  /* Taken as-in from LuaJIT code */
  const char *msg;
  const char *typearg;

  if(luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);
  else
  if(lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";
  else
    typearg = luaL_typename(L, arg);

  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  luaL_argerror(L, arg, msg);
  for(;;);
}

gboolean
_ds_tovalue(lua_State* L,
            gint idx,
            GValue* value,
            GType g_type,
            GError** error);

G_GNUC_INTERNAL
GObject*
_ds_lua_object_new(lua_State *L,
                   GType      g_type,
                   guint      n_properties,
                   GError   **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  GValue* values = NULL;
  const gchar** names = NULL;
  const gchar* name = NULL;
  GObjectClass* klass = NULL;
  GObject* object = NULL;
  GObject* return_ = NULL;
  GParamSpec* pspec = NULL;
  GType type_ = G_TYPE_INVALID;
  guint i, j;

  /* allocate arrays */
  values = (GValue*)
  g_slice_alloc(sizeof(GValue) * n_properties);
  names = (const gchar**)
  g_slice_alloc(sizeof(gchar*) * n_properties);
  memset(values, 0, sizeof(GValue) * n_properties);

  /* take a reference to class */
  klass = g_type_class_ref(g_type);

  for(i = 0, j = 2;
      i < n_properties;
      i++)
  {
    name = lua_tostring(L, j++);
    if G_UNLIKELY(name == NULL)
    {
      g_set_error
      (error,
       DS_LUA_ERROR,
       DS_LUA_ERROR_FAILED,
       "i don't known what happens, take a look at (%s:%s):%i\r\n",
       __FILE__, G_STRFUNC, __LINE__);
      goto_error();
    }

    pspec = g_object_class_find_property(klass, name);
    if G_UNLIKELY(pspec == NULL)
    {
      g_set_error
      (error,
       DS_LUA_ERROR,
       DS_LUA_ERROR_RUNTIME,
       "unknown property %s::%s\r\n",
       g_type_name(g_type), name);
      goto_error();
    }

    type_ = G_PARAM_SPEC_VALUE_TYPE(pspec);
    names[i] = name;

    _ds_tovalue(L, j++, &(values[i]), type_, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_set_error
      (error,
       DS_LUA_ERROR,
       DS_LUA_ERROR_RUNTIME,
       "property %s::%s is of type %s\r\n",
       g_type_name(g_type),
       name,
       g_type_name(type_));
      goto_error();
    }
  }

  object =
  g_object_new_with_properties
  (g_type,
   n_properties,
   names,
   values);

  if G_UNLIKELY(object == NULL)
  {
    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_FAILED,
     "i don't known what happens, take a look at (%s:%s):%i\r\n",
     __FILE__, G_STRFUNC, __LINE__);
    goto_error();
  }

  return_ = g_steal_pointer(&object);

_error_:
  g_clear_pointer
  (&object,
   g_object_unref);
  g_clear_pointer
  (&klass,
   g_type_class_unref);
  if G_LIKELY(names != NULL)
    g_slice_free1
    (sizeof(gchar*)
     * n_properties,
     names);
  if G_LIKELY(values != NULL)
  {
    for(i = 0;
        i < n_properties;
        i++)
    {
      g_value_unset(&(values[i]));
    }

    g_slice_free1
    (sizeof(GValue)
     * n_properties,
     values);
  }
return return_;
}

static int
reporter(lua_State* L)
{
  const gchar* err = lua_tostring(L, 1);
  if G_UNLIKELY(err == NULL)
  {
    if(luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
      err = lua_tostring(L, -1);
    else
      err = lua_pushfstring(L, "(error object is a %s value)", lua_typename(L, 1));
  }

  luaL_traceback(L, L, err, 1);
return 1;
}

gboolean
luaD_xpcall(lua_State  *L,
            int         argc,
            int         retn,
            GError    **error)
{
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  lua_pushcfunction(L, reporter);
  lua_insert(L, 1);

  int return_ =
  lua_pcall(L, argc, retn, 1);
  lua_remove(L, 1);

  switch(return_)
  {
  case LUA_OK:
  case LUA_YIELD:
    /* success! */
    break;
  case LUA_ERRMEM:
    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_MEMORY,
     "out of memory\r\n");
    break;
  case LUA_ERRERR:
    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_RECURSIVE,
     "recursive error\r\n");
    break;
  default:
    {
      const gchar* err =
      lua_tostring(L, -1);

      if G_UNLIKELY(err == NULL)
        err = "unknown error";

      gint code = 0;
      switch(return_)
      {
      case LUA_ERRRUN:
        code = DS_LUA_ERROR_RUNTIME;
        break;
      case LUA_ERRSYNTAX:
        code = DS_LUA_ERROR_SYNTAX;
        break;
      default:
        code = DS_LUA_ERROR_FAILED;
        break;
      }

      g_set_error
      (error,
       DS_LUA_ERROR,
       code,
       err);
    }
    break;
  }
}

/*
 * Lua
 * init / fini
 *
 */

G_GNUC_INTERNAL
gboolean
_ds_lua_init(lua_State  *L,
             GError    **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  luaL_openlibs(L);

  success =
  _ds_lualib_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  lua_settop(L, 0);
return success;
}

G_GNUC_INTERNAL
void
_ds_lua_fini(lua_State  *L)
{
  _ds_lualib_fini(L);
}
