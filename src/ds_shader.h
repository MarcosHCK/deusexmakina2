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
#ifndef __DS_SHADER_INCLUDED__
#define __DS_SHADER_INCLUDED__ 1
#include <ds_export.h>
#include <ds_folder_provider.h>
#include <cglm/cglm.h>
#include <gio/gio.h>

/**
 * DS_SHADER_ERROR:
 *
 * Error domain for DS_SHADER. Errors in this domain will be from the #DsShaderError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_SHADER_ERROR  (ds_shader_error_quark())

/**
 * DsApplicationError:
 * @DS_SHADER_ERROR_FAILED: generic error condition.
 * @DS_SHADER_ERROR_INVALID_BINARY: used internally to
 * implement shader caching.
 * @DS_SHADER_ERROR_COMPILE: error compiling shader.
 * @DS_SHADER_ERROR_LINK: error linking program.
 *
 * Error code returned by DsApplication API.
 * Note that %DS_SHADER_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum {
  DS_SHADER_ERROR_FAILED,
  DS_SHADER_ERROR_INVALID_BINARY,
  DS_SHADER_ERROR_INVALID_SHADER,
  DS_SHADER_ERROR_INVALID_PROGRAM,
  DS_SHADER_ERROR_COMPILE,
  DS_SHADER_ERROR_LINK,
} DsShaderError;

#define DS_TYPE_SHADER            (ds_shader_get_type())
#define DS_SHADER(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_SHADER, DsShader))
#define DS_SHADER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_SHADER, DsShaderClass))
#define DS_IS_SHADER(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_SHADER))
#define DS_IS_SHADER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_SHADER))
#define DS_SHADER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_SHADER, DsShaderClass))

typedef struct _DsShader      DsShader;
typedef struct _DsShaderClass DsShaderClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GQuark
ds_shader_error_quark();
DEUSEXMAKINA2_API
GType
ds_shader_get_type();

struct _DsShaderClass
{
  GObjectClass parent_class;
};

DEUSEXMAKINA2_API
DsShader*
ds_shader_new(GFile            *vertex_file,
              GInputStream     *vertex_stream,
              GFile            *fragment_file,
              GInputStream     *fragment_stream,
              GFile            *geometry_file,
              GInputStream     *geometry_stream,
              DsCacheProvider  *cache_provider,
              GCancellable     *cancellable,
              GError          **error);

DEUSEXMAKINA2_API
DsShader*
ds_shader_new_from_files(GFile           *vertex_file,
                         GFile           *fragment_file,
                         GFile           *geometry_file,
                         DsCacheProvider *cache_provider,
                         GCancellable    *cancellable,
                         GError         **error);

DEUSEXMAKINA2_API
gboolean
ds_shader_use(DsShader     *shader,
              GError      **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_SHADER_INCLUDED__
