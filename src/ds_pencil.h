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
#ifndef __DS_PENCIL_INCLUDED__
#define __DS_PENCIL_INCLUDED__
#include <cglm/cglm.h>
#include <ds_export.h>
#include <ds_renderable.h>
#include <ds_gl.h>

/**
 * DS_PENCIL_ERROR:
 *
 * Error domain for DS_PENCIL. Errors in this domain will be from the #DsPencilError enumeration.
 * See #GError for more information on error domains.
 */
#define DS_PENCIL_ERROR   (ds_pencil_error_quark())

/**
 * DsPencilError:
 * @DS_PENCIL_ERROR_FAILED: generic error condition.
 * @DS_PENCIL_ERROR_CREATED: there are a process-default pencil already.
 *
 * Error code returned by DsPencil API.
 * Note that %DS_PENCIL_ERROR_FAILED is here only for compatibility with
 * error domain definition paradigm as defined on GLib documentation.
 */
typedef enum
{
  DS_PENCIL_ERROR_FAILED,
  DS_PENCIL_ERROR_CREATED,
} DsPencilError;

#define DS_TYPE_PENCIL            (ds_pencil_get_type())
#define DS_PENCIL(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_PENCIL, DsPencil))
#define DS_PENCIL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_PENCIL, DsPencilClass))
#define DS_IS_PENCIL(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_PENCIL))
#define DS_IS_PENCIL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_PENCIL))
#define DS_PENCIL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_PENCIL, DsPencilClass))

typedef struct _DsPencil        DsPencil;
typedef struct _DsPencilClass   DsPencilClass;
typedef struct _DsPencilVertex  DsPencilVertex;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _DsPencilVertex
{
  vec3 position;
  vec3 normal;
  vec3 uvw;
  vec3 tangent;
  vec3 bitangent;
};

DEUSEXMAKINA2_API
GQuark
ds_pencil_error_quark();
DEUSEXMAKINA2_API
GType
ds_pencil_get_type();

DEUSEXMAKINA2_API
DsPencil*
ds_pencil_new(GError** error);
DEUSEXMAKINA2_API
DsPencil*
ds_pencil_get_default();
DEUSEXMAKINA2_API
gboolean
ds_pencil_bind(DsPencil* pencil, GError** error);
DEUSEXMAKINA2_API
void
ds_pencil_switch(DsPencil* pencil, DsRenderState* state, GLuint* p_vbo);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_PENCIL_INCLUDED__
