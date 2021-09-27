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
#ifndef __DS_TEXT_INCLUDED__
#define __DS_TEXT_INCLUDED__ 1
#include <glib-object.h>

/**
 * DS_TEXT_ERROR:
 *
 * Error domain for DS_TEXT. Errors in this domain will be from the #DsTextError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_TEXT_ERROR (ds_text_error_quark())

/**
 * DsApplicationError:
 * @DS_TEXT_ERROR_FAILED: generic error condition.
 *
 * Error code returned by DsText API.
 * Note that %DS_TEXT_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_TEXT_ERROR_FAILED,
} DsTextError;

#define DS_TYPE_TEXT            (ds_text_get_type())
#define DS_TEXT(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_TEXT, DsText))
#define DS_TEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_TEXT, DsTextClass))
#define DS_IS_TEXT(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_TEXT))
#define DS_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_TEXT))
#define DS_TEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_TEXT, DsTextClass))

typedef struct _DsText      DsText;
typedef struct _DsTextClass DsTextClass;
typedef         gpointer    DsTextHandle;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
ds_text_error_quark();
GType
ds_text_get_type();

struct _DsTextClass
{
  GObjectClass parent_class;
};

DsText*
ds_text_new(DsFont* provider);
DsTextHandle
ds_text_print(DsText         *text,
              DsTextHandle    instance,
              const gchar    *text_,
              vec2            position,
              GCancellable   *cancellable,
              GError        **error);
void
ds_text_unprint(DsText         *text,
                DsTextHandle    handle);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_TEXT_INCLUDED__
