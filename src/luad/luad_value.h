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
#ifndef __LUAD_VALUE_INCLUDED__
#define __LUAD_VALUE_INCLUDED__ 1
#include <ds_export.h>
#include <glib-object.h>
#include <luad_core.h>

typedef struct    _LValue       LValue;

typedef gpointer  (*LValueCopy)  (gpointer instance, gpointer metadata);
typedef void      (*LValueFree)  (gpointer instance, gpointer metadata);

#define LVALUE_IS_CLASS(ptr)    (((ptr->classed==1)||(ptr->interface==1))&&(ptr->holds==0))
#define LVALUE_IS_INSTANCE(ptr) (((ptr->classed==1)||(ptr->interface==1))&&(ptr->holds==1))

#if __LUAD_INSIDE__ == 1
#include <girepository.h>

struct _LValue
{
  GType g_type;

  union
  {
    GClosure* callable;
    gint enum_value;
    guint flag_value;
    gpointer struct_ptr;
    gpointer union_ptr;
    gpointer object_ptr;
    gpointer interface_ptr;

    /* generic */
    gpointer instance;
  };

  LValueCopy copy_;
  LValueFree free_;
  gpointer metadata;

  GIBaseInfo* info;

  guint type : 5;         /* type (GIInfoType)      */
  guint fundamental : 1;  /* type is fundamental?   */
  guint classed : 1;      /* type is classed?       */
  guint interface : 1;    /* type is an interface?  */
  guint instantiable : 1; /* type can be instanced  */
  guint taken : 1;        /* held value is static?  */
  guint holds : 1;        /* a value is held?       */
};

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
gboolean
_luaD_lvalue_init(lua_State* L, GError** error);
DEUSEXMAKINA2_API
void
_luaD_lvalue_fini(lua_State* L);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LUAD_INSIDE__

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
LValue*
_luaD_pushlvalue(lua_State* L, GType g_type, gpointer instance);
DEUSEXMAKINA2_API
gboolean
_luaD_islvalue(lua_State* L, int idx);
DEUSEXMAKINA2_API
LValue*
_luaD_tolvalue(lua_State* L, int idx);
DEUSEXMAKINA2_API
LValue*
_luaD_checklvalue(lua_State* L, int arg);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LUAD_VALUE_INCLUDED__
