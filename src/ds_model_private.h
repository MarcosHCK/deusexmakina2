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
#ifndef __DS_MODEL_PRIVATE_INCLUDED__
#define __DS_MODEL_PRIVATE_INCLUDED__ 1
#include <assimp/cimport.h>
#include <assimp/cfileio.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <ds_macros.h>
#include <ds_model.h>

typedef struct _DsModelMesh       DsModelMesh;
typedef struct _DsModelTexture    DsModelTexture;

typedef gboolean (*DsModelTioIterator) (DsModel* model, DsModelTexture* texture, GList* meshes, gpointer user_data);

static const
gint gl2ai[] =
{
  aiTextureType_DIFFUSE,
  aiTextureType_SPECULAR,
  aiTextureType_NORMALS,
  aiTextureType_HEIGHT,
};

struct _DsModelTexture
{
  grefcount refs;

  union
  {
    GLuint tios[(G_N_ELEMENTS(gl2ai))];
    struct
    {
      GLuint diffuse;
      GLuint specular;
      GLuint normal;
      GLuint height;
    };
  };
};

struct _DsModelMesh
{
  GLsizei indices;
  GLuint index_offset;
  GLint base_vertex;
};

#define _aiReleaseImport0(var) ((var == NULL) ? NULL : (var = (aiReleaseImport (var), NULL)))

#if __cplusplus
#endif // __cplusplus

/*
 * ds_model.c
 *
 */

G_GNUC_INTERNAL
void
_ds_model_iterate_tio_groups(DsModel             *self,
                             DsModelTioIterator   foreach_tio,
                             gpointer             user_data);

/*
 * ds_model_buf.c
 *
 */

G_GNUC_INTERNAL
gboolean
_ds_model_make_vao(DsModel   *model,
                   GError   **error);
G_GNUC_INTERNAL
gboolean
_ds_model_compile_switch_vbo(DsModel         *self,
                             DsRenderState   *state,
                             GError         **error);
G_GNUC_INTERNAL
gboolean
_ds_model_compile_switch_ibo(DsModel         *self,
                             DsRenderState   *state,
                             GError         **error);

/*
 * ds_model_imp.c
 *
 */

G_GNUC_INTERNAL
const C_STRUCT aiScene*
_ds_model_import_new(DsModel       *self,
                     GFile         *source,
                     const gchar   *name,
                     GCancellable  *cancellable,
                     GError       **error);
G_GNUC_INTERNAL
void
_ds_model_import_free(DsModel                *self,
                      const C_STRUCT aiScene *scene);

/*
 * ds_model_tex.c
 *
 */

G_GNUC_INTERNAL
GType
ds_model_texture_get_type();
G_GNUC_INTERNAL
DsModelTexture*
ds_model_texture_new(GError** error);
G_GNUC_INTERNAL
DsModelTexture*
ds_model_texture_ref(DsModelTexture* texture);
G_GNUC_INTERNAL
void
ds_model_texture_unref(DsModelTexture* texture);
G_GNUC_INTERNAL
void
_ds_model_compile_switch_texture(DsModelTexture  *tex,
                                 DsRenderState   *state);

#if __cplusplus
#endif // __cplusplus


#endif // __DS_MODEL_PRIVATE_INCLUDED__
