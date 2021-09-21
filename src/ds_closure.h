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
#ifndef __DS_CLOSURE_INCLUDED__
#define __DS_CLOSURE_INCLUDED__
#include <glib-object.h>

/**
 * DS_CLOSURE_ERROR:
 *
 * Error domain for DS_CLOSURE. Errors in this domain will be from the #DsClosureError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_CLOSURE_ERROR (ds_closure_error_quark())

/**
 * DS_CLOSURE_ERROR:
 *
 * See #G_SIGNAL_TYPE_STATIC_SCOPE
 */
#define DS_INVOKE_STATIC_SCOPE G_SIGNAL_TYPE_STATIC_SCOPE

/**
 * DsClosureError:
 * @DS_CLOSURE_ERROR_FAILED: generic error condition.
 * @DS_CLOSURE_ERROR_INVALID_ARGUMENT: invalid argument value supplied to closure or
 * no matching argument number.
 * @DS_CLOSURE_ERROR_INVALID_RETURN: invalid return value supplied to closure.
 *
 * Error code returned by DsClosure API.
 * Note that %DS_CLOSURE_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_CLOSURE_ERROR_FAILED,
  DS_CLOSURE_ERROR_INVALID_ARGUMENT,
  DS_CLOSURE_ERROR_INVALID_RETURN,
} DsClosureError;

/**
 * DsClosureFlags:
 * @DS_CLOSURE_FLAGS_NONE: normal closure.
 * @DS_CLOSURE_CONSTRUCTOR: constructor closure (means you don't have
 * to pass an object instance as first argument).
 *
 * Closure flags
 */
typedef enum {
  DS_CLOSURE_FLAGS_NONE,
  DS_CLOSURE_CONSTRUCTOR,
} DsClosureFlags;

typedef struct _DsClosure DsClosure;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
ds_closure_error_quark();

struct _DsClosure
{
  GClosure closure;
  GCallback callback;
  GVaClosureMarshal vmarshal;
  DsClosureFlags flags;
  GType return_type;
  guint n_params;
  GType* params;
};

gboolean
ds_closure_check_values(DsClosure  *closure,
                        GValue     *return_value,
                        guint       n_params_,
                        GValue     *params,
                        GError    **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_CLOSURE_INCLUDED__
