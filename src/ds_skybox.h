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
#ifndef __DS_SKYBOX_INCLUDED__
#define __DS_SKYBOX_INCLUDED__ 1
#include <ds_export.h>
#include <gio/gio.h>

/**
 * DS_SKYBOX_ERROR:
 *
 * Error domain for DS_SKYBOX. Errors in this domain will be from the #DsSkyboxError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_SKYBOX_ERROR (ds_skybox_error_quark())

/**
 * DsSkyboxError:
 * @DS_SKYBOX_ERROR_INVALID: generic error condition.
 * @DS_SKYBOX_ERROR_TEXTURE_LOAD: error loading texture.
 *
 * Error code returned by DsSkybox API.
 * Note that %DS_SKYBOX_ERROR_INVALID is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_SKYBOX_ERROR_INVALID,
  DS_SKYBOX_ERROR_TEXTURE_LOAD,
} DsSkyboxError;

#define DS_TYPE_SKYBOX            (ds_skybox_get_type())
#define DS_SKYBOX(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_SKYBOX, DsSkybox))
#define DS_SKYBOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_SKYBOX, DsSkyboxClass))
#define DS_IS_SKYBOX(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_SKYBOX))
#define DS_IS_SKYBOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_SKYBOX))
#define DS_SKYBOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_SKYBOX, DsSkyboxClass))

typedef struct _DsSkybox              DsSkybox;
typedef struct _DsSkyboxClass         DsSkyboxClass;
typedef struct _DsSkyboxClassPrivate  DsSkyboxClassPrivate;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GQuark
ds_skybox_error_quark();
DEUSEXMAKINA2_API
GType
ds_skybox_get_type();

DEUSEXMAKINA2_API
DsSkybox*
ds_skybox_new(GFile        *source,
              const gchar  *name,
              GCancellable *cancellable,
              GError      **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_SKYBOX_INCLUDED__
