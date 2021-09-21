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
#ifndef __DS_DDS_INCLUDED__
#define __DS_DDS_INCLUDED__
#include <gio/gio.h>

/**
 * DS_DDS_ERROR:
 *
 * Error domain for DS_DDS. Errors in this domain will be from the #DsDdsError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_DDS_ERROR (ds_dds_error_quark())

/**
 * DsDdsError:
 * @DS_DDS_ERROR_FAILED: generic error condition.
 * @DS_DDS_ERROR_INVALID_HEADER: invalid header value (should be 'DDSx').
 * @DS_DDS_ERROR_INVALID_FOURCC: invalid four character code (should be 'DXTx').
 *
 * Error code returned by #ds_dds_load_image() function.
 * Note that %DS_DDS_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_DDS_ERROR_FAILED,
  DS_DDS_ERROR_INVALID_HEADER,
  DS_DDS_ERROR_INVALID_FOURCC,
  DS_DDS_ERROR_UNSUPPORTED_FORMAT,
} DsDdsError;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
ds_dds_error_quark();

gboolean
ds_dds_load_image(GFile          *file,
                  guint           gltype,
                  GCancellable   *cancellable,
                  GError        **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_DDS_INCLUDED__
