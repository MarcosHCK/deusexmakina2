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
#ifndef __DS_CALLABLE_INCLUDED__
#define __DS_CALLABLE_INCLUDED__
#include <ds_marshals.h>
#include <ds_closure.h>
#include <gio/gio.h>

#define DS_TYPE_CALLABLE            (ds_callable_get_type())
#define DS_CALLABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DS_TYPE_CALLABLE, DsCallable))
#define DS_CALLABLE_CLASS(obj)      (G_TYPE_CHECK_CLASS_CAST((obj), DS_TYPE_CALLABLE, DsCallableIface))
#define DS_IS_CALLABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), DS_TYPE_CALLABLE))
#define DS_CALLABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), DS_TYPE_CALLABLE, DsCallableIface))

typedef struct _DsCallable              DsCallable;
typedef struct _DsCallableIface         DsCallableIface;
typedef struct _DsCallableIfacePrivate  DsCallableIfacePrivate;

#define DS_INVOKE_STATIC_SCOPE G_SIGNAL_TYPE_STATIC_SCOPE

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_callable_get_type();

struct _DsCallableIface
{
  GTypeInterface parent_iface;
  DsCallableIfacePrivate* priv;
};

gboolean
ds_callable_iface_has_field(DsCallableIface* iface,
                            const gchar* name);
DsClosure*
ds_callable_iface_get_field(DsCallableIface* iface,
                            const gchar* name);
void
ds_callable_iface_set_field(DsCallableIface* iface,
                            const gchar* name,
                            DsClosure* closure);
DsCallable*
ds_callable_iface_contructv(DsCallableIface* iface,
                            const gchar* name,
                            GValue* params);
void
ds_callable_iface_add_methodv(DsCallableIface* iface,
                              const gchar* name,
                              DsClosureFlags flags,
                              GCallback callback,
                              GClosureMarshal marshal,
                              GVaClosureMarshal vmarshal,
                              GType return_type,
                              guint n_params,
                              GType* params);
void
ds_callable_iface_add_method_valist(DsCallableIface* iface,
                                    const gchar* name,
                                    DsClosureFlags flags,
                                    GCallback callback,
                                    GClosureMarshal marshal,
                                    GVaClosureMarshal vmarshal,
                                    GType return_type,
                                    guint n_params,
                                    va_list l);
void
ds_callable_iface_add_method(DsCallableIface* iface,
                             const gchar* name,
                             DsClosureFlags flags,
                             GCallback callback,
                             GClosureMarshal marshal,
                             GVaClosureMarshal vmarshal,
                             GType return_type,
                             guint n_params,
                             ...);
gboolean
ds_callable_has_method(DsCallable* callable,
                       const gchar* name);
void
ds_callable_invokev(const GValue* instance_and_params,
                    const gchar* name,
                    GValue* return_value);
void
ds_callable_invoke_valist(DsCallable* callable,
                          const gchar* name,
                          va_list l);
void
ds_callable_invoke(DsCallable* callable,
                   const gchar* name,
                   ...);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_CALLABLE_INCLUDED__
