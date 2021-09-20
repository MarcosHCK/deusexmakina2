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
#include <ds_luaobj.h>
#include <ds_macros.h>
#include <gio/gio.h>

/* Some kind of static assert */
#include <luajit.h>

/*
 * Functions table
 *
 */

gboolean
_ds_tovalue(lua_State* L,
            gint idx,
            GValue* value,
            GType g_type,
            GError** error);

static gboolean
_object_new(lua_State* L,
            GType g_type,
            guint n_properties,
            GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  GValue* values = NULL;
  const gchar** names = NULL;
  const gchar* name = NULL;
  GObjectClass* klass = NULL;
  GObject* object = NULL;
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
       G_IO_ERROR,
       G_IO_ERROR_FAILED,
       "i don't known what happens, take a look at (%s:%s):%i\r\n",
       __FILE__, G_STRFUNC, __LINE__);
      goto_error();
    }

    pspec = g_object_class_find_property(klass, name);
    if G_UNLIKELY(pspec == NULL)
    {
      g_set_error
      (error,
       G_IO_ERROR,
       G_IO_ERROR_FAILED,
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
       G_IO_ERROR,
       G_IO_ERROR_FAILED,
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
     G_IO_ERROR,
     G_IO_ERROR_FAILED,
     "i don't known what happens, take a look at (%s:%s):%i\r\n",
     __FILE__, G_STRFUNC, __LINE__);
    goto_error();
  }

  luaD_pushobject(L, object);

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
return success;
}

static int
object_new(lua_State* L)
{
  GError* tmp_err = NULL;
  GType g_type = G_TYPE_INVALID;
  int top, n_properties;
  const gchar* typename_ =
  luaL_checkstring(L, 1);

  g_type = g_type_from_name(typename_);
  if G_UNLIKELY(g_type == G_TYPE_INVALID)
  {
    lua_pushfstring
    (L,
     "unknown class %s\r\n",
     typename_);
    lua_error(L);
    return 0;
  }

  top = lua_gettop(L) - 1;
  n_properties = top >> 1;

  _object_new(L, g_type, n_properties, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    lua_pushstring(L, tmp_err->message);
    g_error_free(tmp_err);
    lua_error(L);
    return 0;
  }
return 1;
}

static const
luaL_Reg lualib[] =
{
  {"object_new", object_new},
  {NULL, NULL},
};

/*
 * Type table metaindexes
 *
 */

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
      GTypeQuery info;
      g_type_query(g_type, &info);
      if G_LIKELY(info.instance_size > 0)
      {
        luaD_pushclass(L, g_type);
        return 1;
      }
      else
      {
        lua_pushfstring
        (L,
         "%s is an unclassed type\r\n",
         typename_);
        lua_error(L);
        return 0;
      }
    }
  }
return 0;
}

/*
 * luaopen_*
 *
 */

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

void
_ds_lualib_fini(lua_State* L)
{
  _ds_luaobj_fini(L);
  _ds_luaclosure_fini(L);
  _ds_luaclass_fini(L);
  _ds_luaboxed_fini(L);
}
