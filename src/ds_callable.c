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
#include <gobject/gvaluecollector.h>

static
void ds_callable_base_init(DsCallableIface* iface);
static
void ds_callable_base_fini(DsCallableIface* iface);

typedef struct _DsClosure DsClosure;

/*
 * Interface definition
 *
 */

struct _DsClosure
{
  GClosure closure;
  GCallback callback;
  GVaClosureMarshal vmarshal;
  GType return_type;
  guint n_params;
  GType* params;
};

G_STATIC_ASSERT
(G_STRUCT_OFFSET(GCClosure, callback)
 == G_STRUCT_OFFSET(DsClosure, callback));

struct _DsCallableIfacePrivate
{
  GHashTable* funcs;
};

GType
ds_callable_get_type (void)
{
  static
  GType callable_type = 0;

  if G_UNLIKELY(callable_type == 0)
  {
    const
    GTypeInfo type_info = {
      sizeof(DsCallableIface),
      (GBaseInitFunc)
      ds_callable_base_init,
      (GBaseFinalizeFunc)
      ds_callable_base_fini,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL
    };

    callable_type =
    g_type_register_static
    (G_TYPE_INTERFACE,
     "DsCallable",
     &type_info,
     0);
  }
return callable_type;
}

static
void ds_callable_base_init(DsCallableIface* iface) {
  DsCallableIfacePrivate* priv =
  g_slice_new(DsCallableIfacePrivate);
  iface->priv = priv;

  priv->funcs =
  g_hash_table_new_full
  (g_str_hash,
   g_str_equal,
   g_free,
   (GDestroyNotify)
   g_closure_unref);
}

static
void ds_callable_base_fini(DsCallableIface* iface) {
  DsCallableIfacePrivate* priv = iface->priv;
  g_hash_table_remove_all(priv->funcs);
  g_hash_table_unref(priv->funcs);
  g_slice_free(DsCallableIfacePrivate, priv);
}

/*
 * Object methods
 *
 */

gboolean
ds_callable_iface_has_field(DsCallableIface* iface,
                            const gchar* name)
{
  g_return_val_if_fail(iface != NULL, FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  DsCallableIfacePrivate* priv = iface->priv;

  gboolean has =
  g_hash_table_lookup_extended(priv->funcs, name, NULL, NULL);
return has;
}

DsClosure*
ds_callable_iface_get_field(DsCallableIface* iface,
                            const gchar* name)
{
  g_return_val_if_fail(iface != NULL, NULL);
  g_return_val_if_fail(name != NULL, NULL);
  DsCallableIfacePrivate* priv = iface->priv;
return (DsClosure*) g_hash_table_lookup(priv->funcs, name);
}

void
ds_callable_iface_set_field(DsCallableIface* iface,
                            const gchar* name,
                            DsClosure* closure)
{
  g_return_if_fail(iface != NULL);
  g_return_if_fail(name != NULL);
  g_return_if_fail(closure != NULL);
  DsCallableIfacePrivate* priv = iface->priv;

  gboolean has =
  g_hash_table_lookup_extended(priv->funcs, name, NULL, NULL);
  if G_UNLIKELY(has == TRUE)
  {
    g_warning
    ("Trying to set two field with equal names '%s'\r\n",
     name);
    return;
  }

  g_hash_table_insert(priv->funcs, g_strdup(name), closure);

  g_closure_ref((GClosure*) closure);
  g_closure_sink((GClosure*) closure);
}

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static void
_closure_fini0(gpointer notify_data, DsClosure* closure)
{
  _g_free0(closure->params);
}

void
ds_callable_iface_add_methodv(DsCallableIface* iface,
                              const gchar* name,
                              GCallback callback,
                              GClosureMarshal marshal,
                              GVaClosureMarshal vmarshal,
                              GType return_type,
                              guint n_params,
                              GType* params)
{
  g_return_if_fail(callback != NULL);
  if(n_params > 0)
    g_return_if_fail(params != NULL);

  DsClosure* closure = (DsClosure*)
  g_closure_new_simple(sizeof(DsClosure), NULL);
  closure->callback = callback;
  closure->vmarshal = vmarshal;
  closure->return_type = return_type;
  closure->n_params = n_params;
  closure->params = g_memdup(params, sizeof(GType) * n_params);

  g_closure_ref((GClosure*) closure);
  g_closure_sink((GClosure*) closure);

  g_closure_set_marshal
  ((GClosure*) closure,
   marshal);

  g_closure_add_finalize_notifier
  ((GClosure*) closure,
   NULL,
   (GClosureNotify)
   _closure_fini0);

  ds_callable_iface_set_field(iface, name, closure);
  g_closure_unref((GClosure*) closure);
}

void
ds_callable_iface_add_method_valist(DsCallableIface* iface,
                                    const gchar* name,
                                    GCallback callback,
                                    GClosureMarshal marshal,
                                    GVaClosureMarshal vmarshal,
                                    GType return_type,
                                    guint n_params,
                                    va_list l)
{
  /* This is an awesome idea I found in 'g_signal_new_valist' implementation  */
  /* on glib code, just wanna say who write this got a genius spark           */
  GType param_types_stack[200 / sizeof (GType)];
  GType* param_types_heap = NULL;
  GType* param_types;
  guint i;

  param_types = param_types_stack;
  if(n_params > 0)
  {
    if(G_UNLIKELY(n_params > G_N_ELEMENTS(param_types_stack)))
    {
      param_types_heap = g_slice_alloc(sizeof(GType) * n_params);
      param_types = param_types_heap;
    }

    for(i = 0; i < n_params; i++)
      param_types[i] = va_arg(l, GType);
  }

  ds_callable_iface_add_methodv(iface, name, callback, marshal, vmarshal, return_type, n_params, param_types);

  if G_UNLIKELY(param_types_heap != NULL)
  {
    g_slice_free1
    (sizeof(GType) * n_params,
     param_types_heap);
  }
}

void
ds_callable_iface_add_method(DsCallableIface* iface,
                             const gchar* name,
                             GCallback callback,
                             GClosureMarshal marshal,
                             GVaClosureMarshal vmarshal,
                             GType return_type,
                             guint n_params,
                             ...)
{
  g_return_if_fail(name != NULL);

  va_list l;
  va_start(l, n_params);
  ds_callable_iface_add_method_valist(iface, name, callback, marshal, vmarshal, return_type, n_params, l);
  va_end(l);
}

gboolean
ds_callable_has_method(DsCallable* callable,
                       const gchar* name)
{
  g_return_val_if_fail(DS_IS_CALLABLE(callable), FALSE);
  g_return_val_if_fail(name != NULL, FALSE);
  DsCallableIface* iface = DS_CALLABLE_GET_IFACE(callable);
return ds_callable_iface_has_field(iface, name);
}

void
ds_callable_invokev(const GValue* instance_and_params,
                    const gchar* name,
                    GValue* return_value)
{
  g_return_if_fail(instance_and_params != NULL);
  g_return_if_fail(name != NULL);
  DsCallable* callable = g_value_peek_pointer(instance_and_params);
  g_return_if_fail(DS_IS_CALLABLE(callable));

  DsClosure* closure =
  ds_callable_iface_get_field
  (DS_CALLABLE_GET_IFACE(callable),
   name);
  g_return_if_fail(closure != NULL);

  g_closure_invoke
  ((GClosure*)
   closure,
   return_value,
   closure->n_params + 1,
   instance_and_params,
   NULL);
}

void
ds_callable_invoke_valist(DsCallable* callable,
                          const gchar* name,
                          va_list l)
{
  g_return_if_fail(DS_IS_CALLABLE(callable));
  g_return_if_fail(name != NULL);

  DsClosure* closure =
  ds_callable_iface_get_field
  (DS_CALLABLE_GET_IFACE(callable),
   name);
  g_return_if_fail(closure != NULL);
  guint i, n_params = closure->n_params;

  if(closure->vmarshal != NULL)
  {
    GValue return_ = G_VALUE_INIT;
    GType rtype = closure->return_type & ~DS_INVOKE_STATIC_SCOPE;
    gboolean static_scope = closure->return_type & DS_INVOKE_STATIC_SCOPE;

    if(rtype != G_TYPE_NONE)
      g_value_init(&return_, rtype);

    /* use valist marshaler */
    closure->vmarshal
    ((GClosure*)
     closure,
     &return_,
     callable,
     l,
     NULL,
     closure->n_params,
     closure->params);

    if(rtype != G_TYPE_NONE)
    {
      gchar* error = NULL;
      for(i = 0;
          i < n_params;
          i++)
      {
        GType ptype = closure->params[i] & ~DS_INVOKE_STATIC_SCOPE;
        G_VALUE_COLLECT_SKIP(ptype, l);
      }

      G_VALUE_LCOPY
      (&return_,
       l,
       (static_scope == TRUE) ? G_VALUE_NOCOPY_CONTENTS : 0,
       &error);
      if G_LIKELY(error == NULL)
        g_value_unset(&return_);
      else
      {
        g_warning("%s: %s", G_STRLOC, error);
        g_free(error);

        /* At this point glib source gives an excellent           */
        /* explanation for which reason value is leak here        */
        /* See 'g_signal_emit_valist' function code in gsignal.c  */
      }
    }
  }
  else
  {
    /* If glib developers do this this way, */
    /* I will do it too                     */
    GValue* values = g_alloca(sizeof(GValue) * (n_params + 1));
    GValue* params = &(values[1]);
    memset(values, 0, sizeof(GValue) * (n_params + 1));

    for(i = 0;
        i < n_params;
        i++)
    {
      gchar* error = NULL;
      GType ptype = closure->params[i] & ~DS_INVOKE_STATIC_SCOPE;
      gboolean static_scope = closure->params[i] & DS_INVOKE_STATIC_SCOPE;

      G_VALUE_COLLECT_INIT
      (&(params[i]),
       ptype,
       l,
       (static_scope == TRUE) ? G_VALUE_NOCOPY_CONTENTS : 0,
       &error);
      if G_UNLIKELY(error != NULL)
      {
        g_warning("%s: %s", G_STRLOC, error);
        g_free(error);

        /* At this point glib source gives an excellent           */
        /* explanation for which reason value is leak here        */
        /* See 'g_signal_emit_valist' function code in gsignal.c  */

        while(i--)
          g_value_unset(&(params[i]));
        return;
      }
    }

    values->g_type = G_TYPE_INVALID;
    g_value_init_from_instance(values, callable);

    if(closure->return_type == G_TYPE_NONE)
    {
      ds_callable_invokev(values, name, NULL);
    }
    else
    {
      gchar* error = NULL;
      GType rtype = closure->return_type & ~DS_INVOKE_STATIC_SCOPE;
      gboolean static_scope = closure->return_type & DS_INVOKE_STATIC_SCOPE;
      GValue return_ = G_VALUE_INIT;

      g_value_init(&return_, rtype);
      ds_callable_invokev(values, name, &return_);

      G_VALUE_LCOPY
      (&return_,
       l,
       (static_scope == TRUE) ? G_VALUE_NOCOPY_CONTENTS : 0,
       &error);
      if G_LIKELY(error == NULL)
        g_value_unset(&return_);
      else
      {
        g_warning("%s: %s", G_STRLOC, error);
        g_free(error);

        /* At this point glib source gives an excellent           */
        /* explanation for which reason value is leak here        */
        /* See 'g_signal_emit_valist' function code in gsignal.c  */
      }
    }

    for(i = 0;
        i < n_params;
        i++)
    {
      g_value_unset(&(params[i]));
    }

    g_value_unset(values);
  }
}

void
ds_callable_invoke(DsCallable* callable,
                   const gchar* name,
                   ...)
{
  va_list l;
  va_start(l, name);
  ds_callable_invoke_valist(callable, name, l);
  va_end(l);
}
