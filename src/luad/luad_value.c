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
#include <ds_macros.h>
#include <girepository.h>
#include <luad_closure.h>
#include <luad_lib.h>
#include <luad_value.h>

#define _g_base_info_ref0(var) ((var == NULL) ? NULL : (var = (g_base_info_ref ((GIBaseInfo*) var))))
#define _g_base_info_unref0(var) ((var == NULL) ? NULL : (var = (g_base_info_unref ((GIBaseInfo*) var), NULL)))

#define _METATABLE "LValue"

/*
 * Utils
 *
 */

gpointer
custom_get_function_ptr(GIObjectInfo* info, const gchar* (*getter) (GIObjectInfo*))
{
  GIBaseInfo* parent = NULL;
  const gchar* name = NULL;
  gpointer func = NULL;

  _g_base_info_ref0(info);

  do
  {
    name = getter(info);
    if(name && g_typelib_symbol(g_base_info_get_typelib(info), name, &func))
    {
      _g_base_info_unref0(info);
      break;
    }

    parent = g_object_info_get_parent(info);
    _g_object_unref0(info);
    info = parent;
  }
  while(info != NULL);
return func;
}

static gpointer
gi_type_boxed_copy(gpointer instance, GType g_type)
{
  return g_boxed_copy(g_type, instance);
}

static void
gi_type_boxed_free(gpointer instance, GType g_type)
{
  g_boxed_free(g_type, instance);
}

static gpointer
gi_type_struct_copy(gpointer instance, gsize size)
{
  return g_memdup2(instance, size);
}

static void
gi_type_struct_free(gpointer instance, gsize size)
{
  g_free(instance);
}

static gpointer
gi_type_static_copy(gpointer instance)
{
  return instance;
}

static void
gi_type_static_free(gpointer instance)
{
}

/*
 * Public API
 *
 */

LValue*
_luaD_pushlvalue(lua_State* L, GType g_type, gpointer instance)
{
  GIRepository* repo = NULL;
  GIBaseInfo* base = NULL;
  GType fund = G_TYPE_INVALID;
  GIInfoType type = 0;
  LValue* ptr = NULL;

/*
 * Allocate userdata value
 *
 */

  ptr = (LValue*)
  lua_newuserdata(L, sizeof(LValue));
  luaL_setmetatable(L, _METATABLE);
  ptr->g_type = g_type;

/*
 * Get type info
 *
 */

  repo = g_irepository_get_default();
  base = g_irepository_find_by_gtype(repo, g_type);

  if G_UNLIKELY(base == NULL)
  {
    fund = G_TYPE_FUNDAMENTAL(g_type);
    base = g_irepository_find_by_gtype(repo, fund);

    if G_UNLIKELY(base == NULL)
    {
      if(fund == G_TYPE_OBJECT)
        type = GI_INFO_TYPE_OBJECT;
      else
      if(fund == G_TYPE_INTERFACE)
        type = GI_INFO_TYPE_INTERFACE;
      else
      if(fund == G_TYPE_VARIANT
      || G_TYPE_IS_BOXED(g_type)
      || G_TYPE_IS_PARAM(g_type)
      || G_TYPE_IS_VALUE(g_type))
        type = GI_INFO_TYPE_STRUCT;
      else
      {
        lua_pushfstring
        (L,
         "Can't introspect type '%s'\r\n",
         g_type_name(g_type));
        _luaD_throw_lerror(L);
      }

      ptr->info = NULL;
      ptr->type = type;
    }
    else
    {
      ptr->info = g_base_info_ref(base);
      ptr->type = g_base_info_get_type(base);
      type = ptr->type;
    }
  }
  else
  {
    ptr->info = g_base_info_ref(base);
    ptr->type = g_base_info_get_type(base);
    type = ptr->type;
  }

/*
 * Select copy and free (usually ref and unref) functions
 *
 */

  ptr->copy_ = NULL;
  ptr->free_ = NULL;
  ptr->metadata = NULL;

  switch(type)
  {
  case GI_INFO_TYPE_STRUCT:
    G_GNUC_FALLTHROUGH;
  case GI_INFO_TYPE_UNION:
    if(G_TYPE_IS_BOXED(g_type))
    {
      ptr->copy_ = (LValueCopy) gi_type_boxed_copy;
      ptr->free_ = (LValueFree) gi_type_boxed_free;
      ptr->metadata = (gpointer) g_type;
    }
    else
    {
      ptr->copy_ = (LValueCopy) gi_type_struct_copy;
      ptr->free_ = (LValueFree) gi_type_struct_free;
      ptr->metadata = (gpointer)
      g_struct_info_get_size((GIStructInfo*) base);
    }
    break;
  case GI_INFO_TYPE_ENUM:
    G_GNUC_FALLTHROUGH;
  case GI_INFO_TYPE_FLAGS:
    ptr->copy_ = (LValueCopy) gi_type_static_copy;
    ptr->free_ = (LValueFree) gi_type_static_free;
    break;
  case GI_INFO_TYPE_OBJECT:
    G_GNUC_FALLTHROUGH;
  case GI_INFO_TYPE_INTERFACE:
    ptr->copy_ = (LValueCopy) g_object_ref;
    ptr->free_ = (LValueFree) g_object_unref;
    break;
  }

  if G_UNLIKELY(ptr->copy_ == NULL)
  {
    if(base != NULL && g_object_info_get_fundamental(base))
    {
      GIObjectInfoRefFunction copy_ =
      custom_get_function_ptr(base, g_object_info_get_ref_function);
      if(copy_ != NULL)
        ptr->copy_ = (LValueCopy) copy_;
    }
  }

  if G_UNLIKELY(ptr->free_ == NULL)
  {
    if(base != NULL && g_object_info_get_fundamental(base))
    {
      GIObjectInfoUnrefFunction free_ =
      custom_get_function_ptr(base, g_object_info_get_unref_function);
      if(free_ != NULL)
        ptr->free_ = (LValueFree) free_;
    }
  }

/*
 * Do copy
 *
 */

  if(instance != NULL)
  {
    ptr->instance = ptr->copy_(instance, ptr->metadata);
    ptr->taken = TRUE;
    ptr->holds = TRUE;
  }
  else
  {
    ptr->instance = NULL;
    ptr->taken = FALSE;
    ptr->holds = FALSE;
  }

/*
 * Pick other flags
 *
 */

  ptr->fundamental = G_TYPE_IS_FUNDAMENTAL(g_type);
  ptr->classed = G_TYPE_IS_CLASSED(g_type);
  ptr->interface = G_TYPE_IS_INTERFACE(g_type);
  ptr->instantiable = G_TYPE_IS_INSTANTIATABLE(g_type);

/*
 * Clean-up
 *
 */

  _g_base_info_unref0(base);
return ptr;
}

gboolean
_luaD_islvalue(lua_State* L, int idx)
{
  LValue* ptr =
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

LValue*
_luaD_tolvalue(lua_State* L, int idx)
{
  LValue* ptr =
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

LValue*
_luaD_checklvalue(lua_State* L, int arg)
{
  gboolean is =
  _luaD_islvalue(L, arg);
  if G_UNLIKELY(is == FALSE)
    _luaD_typeerror(L, arg, _METATABLE);
return _luaD_tolvalue(L, arg);
}


/*
 * Metatable
 *
 */

static int
__tostring(lua_State* L)
{
  LValue* ptr =
  _luaD_checklvalue(L, 1);

  lua_pushstring(L, g_type_name(ptr->g_type));
  if(LVALUE_IS_INSTANCE(ptr))
  {
    lua_pushfstring(L, " %p", (guintptr) ptr->instance);
    lua_concat(L, 2);
  }
return 1;
}

static gboolean
__newindex_p(lua_State* L, LValue* ptr, const gchar* t, GError** error)
{
  gboolean success = FALSE;
  GError* tmp_err = NULL;

_error_:
return success;
}

static int
__newindex(lua_State* L)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  const gchar* t = NULL;
  LValue* ptr = NULL;

  ptr =
  _luaD_checklvalue(L, 1);
  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    success =
    __newindex_p(L, ptr, t, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
      _luaD_throw_gerror(L, tmp_err);
    return (success == TRUE) ? 1 : 0;
  }
return 0;
}

static gboolean
__index_p(lua_State* L, LValue* ptr, const gchar* t, GError** error)
{
  gboolean success = FALSE;
  GError* tmp_err = NULL;

_error_:
return success;
}

static int
__index(lua_State* L)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  const gchar* t = NULL;
  LValue* ptr = NULL;

  ptr =
  _luaD_checklvalue(L, 1);
  if G_LIKELY
    (lua_isstring(L, 2)
     && (t = lua_tostring(L, 2)) != NULL)
  {
    success =
    __index_p(L, ptr, t, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
      _luaD_throw_gerror(L, tmp_err);
    return (success == TRUE) ? 1 : 0;
  }
return 0;
}

static int
__gc(lua_State* L)
{
  LValue* ptr =
  _luaD_checklvalue(L, 1);
  if(ptr->holds && ptr->taken)
  {
    if G_LIKELY
      (ptr->instance != NULL)
    {
      ptr->free_(ptr->instance, ptr->metadata);
    }
  }

  _g_base_info_unref0(ptr->info);
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

/*
 * Module
 *
 */

gboolean
_luaD_lvalue_init(lua_State* L, GError** error)
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
_luaD_lvalue_fini(lua_State* L)
{
}
