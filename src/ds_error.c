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
#include <ds_error.h>

static
void ds_error_ds_callable_iface_init(DsCallableIface* iface);

/*
 * Object definition
 *
 */

G_DEFINE_TYPE_WITH_CODE
(DsError,
 ds_error,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_CALLABLE,
  ds_error_ds_callable_iface_init));

static DsError*
_callable_new(gpointer null_)
{
  return (DsError*)
  g_object_new
  (DS_TYPE_ERROR,
   NULL);
}

static gpointer
_callable_ref(DsError* self)
{
  return &(self->error);
}

static void
_callable_check(DsError* self)
{
  if G_UNLIKELY(self->error != NULL)
  {
    g_critical(self->error->message);
  }
}

static
void ds_error_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new),
   ds_cclosure_marshal_OBJECT__VOID,
   ds_cclosure_marshal_OBJECT__VOIDv,
   DS_TYPE_ERROR,
   0,
   G_TYPE_NONE);

  ds_callable_iface_add_method
  (iface,
   "ref",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(_callable_ref),
   ds_cclosure_marshal_POINTER__VOID,
   ds_cclosure_marshal_POINTER__VOIDv,
   G_TYPE_POINTER,
   0,
   G_TYPE_NONE);

  ds_callable_iface_add_method
  (iface,
   "check",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(_callable_check),
   g_cclosure_marshal_VOID__VOID,
   g_cclosure_marshal_VOID__VOIDv,
   G_TYPE_NONE,
   0,
   G_TYPE_NONE);
}

static
void ds_error_class_finalize(GObject* pself) {
  DsError* self = DS_ERROR(pself);
  GError* error = self->error;

  if G_UNLIKELY(error != NULL)
  {
    g_warning
    ("uncaught error: %s: %i: %s\r\n",
     g_quark_to_string(error->domain),
     error->code,
     error->message);
    g_error_free(error);
  }

G_OBJECT_CLASS(ds_error_parent_class)->finalize(pself);
}

static
void ds_error_class_init(DsErrorClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  oclass->finalize = ds_error_class_finalize;
}

static
void ds_error_init(DsError* self) {
}
