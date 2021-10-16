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

#define checktype(g_type, expected) \
  G_STMT_START { \
    if G_UNLIKELY \
      (g_type_is_a((g_type), (expected)) == FALSE) \
    { \
      g_critical \
      ("(%s: %i): Trying to use a %s as %s\r\n", \
       G_STRFUNC, \
       __LINE__, \
       g_type_name(g_type), \
       g_type_name(expected)); \
      lua_pushnil(L); \
      return; \
    } \
  } G_STMT_END

/*
 * objects
 *
 */

void
(luaD_pushobject)(lua_State* L, GObject* object)
{
  GType g_type =
  (object != NULL) ? G_TYPE_FROM_INSTANCE(object) : G_TYPE_OBJECT;
#if DEVELOPER
  /* sanity check */
  checktype(g_type, G_TYPE_OBJECT);
#endif // DEVELOPER
  DsGType* ptr =
  _luaD_pushgtype(L, g_type, _g_object_ref_sink0(object));
  ptr->notify = g_object_unref;
  ptr->instanced = TRUE;
}

gboolean
(luaD_isobject)(lua_State* L, int idx)
{
  if G_LIKELY
    (_luaD_isgtype(L, idx) == TRUE)
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if G_LIKELY
      (g_type_is_a(ptr->g_type, G_TYPE_OBJECT)
       && ptr->instanced == TRUE)
    {
      return TRUE;
    }
  }
return FALSE;
}

GObject*
(luaD_toobject)(lua_State* L, int idx)
{
  if G_LIKELY
    (luaD_isobject(L, idx))
  {
    return (GObject*)
    _luaD_togtype(L, idx)->instance;
  }
return NULL;
}

GObject*
(luaD_checkobject)(lua_State* L, int idx)
{
  if G_LIKELY
    (luaD_isobject(L, idx))
  {
    return (GObject*)
    _luaD_togtype(L, idx)->instance;
  }
  else
  {
    _ds_lua_typeerror(L, idx, "GObject");
  }
return NULL;
}

/*
 * class
 *
 */

void
(luaD_pushclass)(lua_State* L, GType g_type)
{
#if DEVELOPER
  /* sanity check */
  checktype(g_type, G_TYPE_OBJECT);
#endif // DEVELOPER
  DsGType* ptr =
  _luaD_pushgtype(L, g_type, NULL);

  if(ptr->classed == TRUE)
  {
    ptr->klass = g_type_class_ref(g_type);
    ptr->notify = g_type_class_unref;
  }
}

gboolean
(luaD_isclass)(lua_State* L, int idx)
{
  if G_LIKELY
    (_luaD_isgtype(L, idx) == TRUE)
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if G_LIKELY
      (g_type_is_a(ptr->g_type, G_TYPE_OBJECT)
       && ptr->instanced == FALSE)
    {
      return TRUE;
    }
  }
return FALSE;
}

GObjectClass*
(luaD_toclass)(lua_State* L, int idx)
{
  if G_LIKELY
    (luaD_isclass(L, idx))
  {
    return (GObjectClass*)
    _luaD_togtype(L, idx)->klass;
  }
return NULL;
}

GObjectClass*
(luaD_checkclass)(lua_State* L, int idx)
{
  if G_LIKELY
    (luaD_isobject(L, idx))
  {
    return (GObjectClass*)
    _luaD_togtype(L, idx)->klass;
  }
  else
  {
    _ds_lua_typeerror(L, idx, "GObjectClass");
  }
return NULL;
}

/*
 * boxed
 *
 */

void
(luaD_pushboxed)(lua_State* L, GType g_type, gpointer instance)
{
#if DEVELOPER
  /* sanity check */
  checktype(g_type, G_TYPE_BOXED);
#endif // DEVELOPER
  if(instance != NULL)
    instance =
    g_boxed_copy(g_type, instance);

  DsGType* ptr =
  _luaD_pushgtype(L, g_type, instance);
  ptr->notify = NULL;
}

gboolean
(luaD_isboxed)(lua_State* L, int idx)
{
  if G_LIKELY
    (_luaD_isgtype(L, idx) == TRUE)
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if G_LIKELY
      (g_type_is_a(ptr->g_type, G_TYPE_BOXED)
       && ptr->instanced == TRUE)
    {
      return TRUE;
    }
  }
return FALSE;
}

gpointer
(luaD_toboxed)(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isboxed(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->boxed;
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return NULL;
}

gpointer
luaD_checkboxed(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isboxed(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->boxed;
  }
  else
  {
    _ds_lua_typeerror(L, idx, "GBoxed");
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return NULL;
}

/*
 * enumerations
 *
 */

void
(luaD_pushenum)(lua_State* L, GType g_type, gint value)
{
#if DEVELOPER
  /* sanity check */
  checktype(g_type, G_TYPE_ENUM);
#endif // DEVELOPER
  DsGType* ptr =
  _luaD_pushgtype(L, g_type, GINT_TO_POINTER(value));
  ptr->instanced = TRUE;
}

gboolean
(luaD_isenum)(lua_State* L, int idx)
{
  if G_LIKELY
    (_luaD_isgtype(L, idx) == TRUE)
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if G_LIKELY
      (g_type_is_a(ptr->g_type, G_TYPE_ENUM)
       && ptr->instanced == TRUE)
    {
      return TRUE;
    }
  }
return FALSE;
}

gint
(luaD_toenum)(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isenum(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->enum_value;
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return -1;
}

gint
(luaD_checkenum)(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isenum(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->enum_value;
  }
  else
  {
    _ds_lua_typeerror(L, idx, "GEnum");
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return -1;
}

/*
 * flags
 *
 */

void
(luaD_pushflags)(lua_State* L, GType g_type, guint value)
{
#if DEVELOPER
  /* sanity check */
  checktype(g_type, G_TYPE_FLAGS);
#endif // DEVELOPER
  DsGType* ptr =
  _luaD_pushgtype(L, g_type, GUINT_TO_POINTER(value));
  ptr->instanced = TRUE;
}

gboolean
(luaD_isflags)(lua_State* L, int idx)
{
  if G_LIKELY
    (_luaD_isgtype(L, idx) == TRUE)
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if G_LIKELY
      (g_type_is_a(ptr->g_type, G_TYPE_FLAGS)
       && ptr->instanced == TRUE)
    {
      return TRUE;
    }
  }
return FALSE;
}

guint
(luaD_toflags)(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isflags(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->flags_value;
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return 0;
}

guint
(luaD_checkflags)(lua_State* L, int idx, GType* g_type)
{
  if G_LIKELY
    (luaD_isflags(L, idx))
  {
    DsGType* ptr =
    _luaD_togtype(L, idx);
    if(g_type)
      *g_type = ptr->g_type;
    return ptr->flags_value;
  }
  else
  {
    _ds_lua_typeerror(L, idx, "GFlags");
  }

  if(g_type)
    *g_type = G_TYPE_INVALID;
return 0;
}

/*
 * generic
 *
 */

void
(luaD_pushgtype)(lua_State* L, GType g_type)
{
  if(G_TYPE_IS_ENUM(g_type))
    luaD_pushenum(L, g_type, -1);
  else
  if(G_TYPE_IS_FLAGS(g_type))
    luaD_pushflags(L, g_type, 0);
  else
  if(G_TYPE_IS_BOXED(g_type))
    luaD_pushboxed(L, g_type, NULL);
  else
  if(   G_TYPE_IS_OBJECT(g_type)
     || G_TYPE_IS_INTERFACE(g_type))
    luaD_pushclass(L, g_type);
  else
  {
    g_critical
    ("Unknown conversion for type %s\r\n",
     g_type_name(g_type));
  }

}
