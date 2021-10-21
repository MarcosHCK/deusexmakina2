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
#include <ds_dds.h>
#include <ds_model_private.h>
#include <ds_mvpholder.h>
#include <jit/jit.h>

/**
 * SECTION:dsmodel
 * @Short_description: Base class for model object
 * @Title: DsModel
 *
 * DsModel is a base class for GL-compatible model
 * loading. This class loads static (no animated)
 * model shipped using a large number of 3d model
 * formats (uses Assimp for that) and converts it
 * to an intermediate data format to be easily use
 * by derived classes.
 *
 */

G_DEFINE_QUARK(ds-model-error-quark,
               ds_model_error);

static void
ds_model_base_class_init(DsModelClass* klass);
static void
ds_model_base_class_fini(DsModelClass* klass);
static void
ds_model_class_init(DsModelClass* klass);
static void
ds_model_init(DsModel* self);
static void
ds_model_ds_mvp_holder_iface_init(DsMvpHolderIface* iface);
static void
ds_model_ds_renderable_iface_init(DsRenderableIface* iface);
static void
ds_model_g_initable_iface_init(GInitableIface* iface);
static
gpointer ds_model_parent_class = NULL;
static
gint DsModel_private_offset;

typedef union  _DsModelTioArray   DsModelTioArray;
typedef union  _DsModelMeshArray  DsModelMeshArray;
typedef struct _DsModelTio        DsModelTio;

/* type safety */
#if defined(glib_typeof) && GLIB_VERSION_MAX_ALLOWED >= GLIB_VERSION_2_58 && (!defined(glib_typeof_2_68) || GLIB_VERSION_MIN_REQUIRED >= GLIB_VERSION_2_68)
# define ds_array_ref(var) ((glib_typeof (var)) (ds_array_ref) (var))
#else  /* __GNUC__ */
/* This version does not depend on gcc extensions, but gcc does not warn
 * about incompatible-pointer-types: */
#define ds_array_ref(var) \
  ((var == NULL) ? NULL : (var = ((ds_array_ref) (var))))
#endif /* __GNUC__ */

#define ds_array_unref(var) \
  ((var == NULL) ? NULL : (var = ((ds_array_unref) (var), NULL)))
#define ds_list_free(var) \
  ((var == NULL) ? NULL : (var = ((ds_list_free) (var), NULL)))

G_STATIC_ASSERT(sizeof(guint) >= sizeof(GLuint));
G_STATIC_ASSERT(sizeof(DsModelIndex) == sizeof(GLuint));

static const
gchar* tex_uniforms[] =
{
  "a_Diffuse",
  "a_Specular",
  "a_Normal",
  "a_Height",
};

G_STATIC_ASSERT(G_N_ELEMENTS(tex_uniforms) == G_N_ELEMENTS(gl2ai));

/*
 * Object definition
 *
 */

struct _DsModelPrivate
{
  GFile* source;
  gchar* filename;

  gboolean notified;

  vec3 scale;
  vec3 position;
  mat4 model;

  union _DsModelTioArray
  {
    GArray array_;
    struct
    {
      DsModelTio* a;
      guint len;
    };
  } *tios;

  union _DsModelMeshArray
  {
    GArray array_;
    struct
    {
      DsModelMesh* a;
      guint len;
    };
  } *meshes;

};

struct _DsModelTio
{
  DsModelTexture* tex;
  union _DsModelMeshList
  {
    GList list_;
    struct
    {
      DsModelMesh* mesh;
      union _DsModelMeshList* next;
      union _DsModelMeshList* prev;
    };
  } *meshes;
};

enum {
  prop_0,
  prop_source,
  prop_name,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

_G_DEFINE_TYPE_EXTENDED_CLASS_INIT(DsModel, ds_model);

GType
ds_model_get_type()
{
  static
  GType g_type = 0;
  if G_UNLIKELY(g_type == 0)
  {
    const GTypeInfo
    type_info =
    {
      sizeof(DsModelClass),
      (GBaseInitFunc)
      ds_model_base_class_init,
      (GBaseFinalizeFunc)
      ds_model_base_class_fini,
      (GClassInitFunc)
      ds_model_class_intern_init,
      NULL,
      NULL,
      sizeof(DsModel),
      0,
      (GInstanceInitFunc)
      ds_model_init,
      NULL,
    };

    g_type =
    g_type_register_static(G_TYPE_OBJECT, g_intern_static_string("DsModel"), &type_info, G_TYPE_FLAG_ABSTRACT);

#define g_define_type_id g_type

    G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE, ds_model_g_initable_iface_init)
    G_IMPLEMENT_INTERFACE(DS_TYPE_MVP_HOLDER, ds_model_ds_mvp_holder_iface_init)
    G_IMPLEMENT_INTERFACE(DS_TYPE_RENDERABLE, ds_model_ds_renderable_iface_init)

    G_ADD_PRIVATE(DsModel);

#undef g_define_type_id
  }
return g_type;
}

static gpointer
(ds_array_ref)(gpointer g_array)
{
  return g_array_ref(g_array);
}

static void
(ds_array_unref)(gpointer g_array)
{
  g_array_unref(g_array);
}

static void
(ds_list_free)(gpointer g_list)
{
  g_list_free(g_list);
}

static void
_tio_clear0(DsModelTio* tio)
{
  g_clear_pointer(&(tio->meshes), ds_list_free);
  g_clear_pointer(&(tio->tex), ds_model_texture_unref);
}

static void
_mesh_clear0(DsModelMesh* mesh)
{
}

static inline void
copy_vertex(DsModel                *self,
            const C_STRUCT aiMesh  *mesh,
            guint                   i,
            DsModelVertex          *v)
{
  /* position */
  v->position[0] = mesh->mVertices[i].x;
  v->position[1] = mesh->mVertices[i].y;
  v->position[2] = mesh->mVertices[i].z;

  /* normal */
  if(mesh->mNormals != NULL)
  {
    v->normal[0] = mesh->mNormals[i].x;
    v->normal[1] = mesh->mNormals[i].y;
    v->normal[2] = mesh->mNormals[i].z;
  }

  /* textures */
  if(mesh->mTextureCoords != NULL)
  {
    /* uvw */
    switch(mesh->mNumUVComponents[0])
    {
    case 3:
      v->uvw[2] = mesh->mTextureCoords[0][i].z;
      G_GNUC_FALLTHROUGH;
    case 2:
      v->uvw[1] = mesh->mTextureCoords[0][i].y;
      G_GNUC_FALLTHROUGH;
    case 1:
      v->uvw[0] = mesh->mTextureCoords[0][i].x;
      break;
    default:
      g_critical
      ("(%s: %i): mesh->mNumUVComponents[i] = %u\r\n",
       G_STRFUNC, __LINE__, mesh->mNumUVComponents[i]);
      g_assert_not_reached();
      break;
    }

    if(mesh->mTangents != NULL)
    {
      v->tangent[0] = mesh->mTangents[i].x;
      v->tangent[1] = mesh->mTangents[i].y;
      v->tangent[2] = mesh->mTangents[i].z;
    }

    if(mesh->mBitangents != NULL)
    {
      v->bitangent[0] = mesh->mBitangents[i].x;
      v->bitangent[1] = mesh->mBitangents[i].y;
      v->bitangent[2] = mesh->mBitangents[i].z;
    }
  }
}

static gboolean
translate_material(DsModel* self, C_STRUCT aiMaterial* material, C_ENUM aiTextureType type, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  C_STRUCT aiString name = {0};
  GFile* child = NULL;
  guint i;

  guint n_images = aiGetMaterialTextureCount(material, type);
  if(n_images == 0) return success;

  guint b_images = sizeof(DsDds*) * n_images;
  DsDds** images = g_alloca(b_images);
#if HAVE_MEMSET == 1
  memset(images, 0, b_images);
#else
  for(i = 0;
      i < n_images;
      i++)
  {
    images[i] = NULL;
  }
#endif // HAVE_MEMSET

  guint width = 0;
  guint height = 0;
  guint n_mipmap = 0;

  for(i = 0;
      i < n_images;
      i++)
  {
  /*
   * Get image file path
   *
   */

    aiGetMaterialTexture(material, type, i, &name, NULL, NULL, NULL, NULL, NULL, NULL);
    g_assert(name.length > 0);

    _g_object_unref0(child);
    child = g_file_get_child(self->priv->source, name.data);

  /*
   * Actually load image
   *
   */

    images[i] =
    ds_dds_new(child, cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }

  /*
   * Check consistency across images
   *
   */

    if G_UNLIKELY(width == 0)
      width = ds_dds_get_width(images[i]);
    else
    if G_UNLIKELY(width != ds_dds_get_width(images[i]))
    {
      g_set_error_literal
      (error,
       DS_MODEL_ERROR,
       DS_MODEL_ERROR_TEXTURE_LOAD,
       "Incompatible width values for multi-layer texture\r\n");
      goto_error();
    }

    if G_UNLIKELY(height == 0)
      height = ds_dds_get_height(images[i]);
    else
    if G_UNLIKELY(height != ds_dds_get_height(images[i]))
    {
      g_set_error_literal
      (error,
       DS_MODEL_ERROR,
       DS_MODEL_ERROR_TEXTURE_LOAD,
       "Incompatible height values for multi-layer texture\r\n");
      goto_error();
    }

    if G_UNLIKELY(n_mipmap == 0)
      n_mipmap = ds_dds_get_n_mipmap(images[i]);
    else
    if G_UNLIKELY(n_mipmap != ds_dds_get_n_mipmap(images[i]))
    {
      g_set_error_literal
      (error,
       DS_MODEL_ERROR,
       DS_MODEL_ERROR_TEXTURE_LOAD,
       "Incompatible mipmap levels for multi-layer texture\r\n");
      goto_error();
    }
  }

  __gl_try_catch(
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, n_mipmap, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, width, height, n_images);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  for(i = 0;
      i < n_images;
      i++)
  {
    success =
    ds_dds_load_into_texture_3d(images[i], FALSE, GL_TEXTURE_2D_ARRAY, i, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

  __gl_try_catch(
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  ,
    g_propagate_error(error, glerror);
    goto_error()
  );

_error_:
  _g_object_unref0(child);
  for(i = 0;
      i < n_images;
      i++)
  {
    _g_object_unref0(images[i]);
  }
return success;
}

static inline DsModelTexture*
load_texture_file(DsModel* self, C_STRUCT aiMaterial* material, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsModelTexture* tex = NULL;
  guint i;

  tex =
  ds_model_texture_new(&tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  for(i = 0;
      i < G_N_ELEMENTS(gl2ai);
      i++)
  {
    __gl_try_catch
    (
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D_ARRAY, tex->tios[i]);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    success =
    translate_material(self, material, gl2ai[i], cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }

    __gl_try_catch
    (
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
      glActiveTexture(GL_TEXTURE0);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    g_clear_pointer(&tex, ds_model_texture_unref);
return tex;
}

static inline gboolean
load_object_file(DsModel* self, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  const C_STRUCT aiScene* scene = NULL;
  guint i, j, k, _v, _i;

  GLuint vbo = 0, ibo = 0;
  DsModelTioArray* tios = NULL;
  DsModelMeshArray* meshes = NULL;
  C_STRUCT aiMesh* mesh = NULL;
  C_STRUCT aiFace* face = NULL;
  DsModelVertex* vertices = NULL;
  DsModelIndex* indices = NULL;

/*
 * Load scene
 *
 */

  scene =
  _ds_model_import_new(self, self->priv->source, self->priv->filename, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Prepare buffers
 *
 */

  /* create buffers object */
  __gl_try_catch(
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ibo);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  /* calculate buffers size */
  gsize n_vertices = 0;
  gsize n_indices = 0;

  for(i = 0;
      i < scene->mNumMeshes;
      i++)
  {
    n_vertices += scene->mMeshes[i]->mNumVertices;

    for(j = 0;
        j < scene->mMeshes[i]->mNumFaces;
        j++)
    {
      n_indices += scene->mMeshes[i]->mFaces[j].mNumIndices;
    }
  }

  /* allocate buffers */
  __gl_try_catch(
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof(DsModelVertex), NULL, GL_STATIC_DRAW);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices * sizeof(DsModelIndex), NULL, GL_STATIC_DRAW);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  tios = (DsModelTioArray*) g_array_new(FALSE, TRUE, sizeof(DsModelTio));
  g_array_set_size(&(tios->array_), scene->mNumMaterials);
  g_array_set_clear_func(&(tios->array_), (GDestroyNotify) _tio_clear0);
  meshes = (DsModelMeshArray*) g_array_new(FALSE, TRUE, sizeof(DsModelMesh));
  g_array_set_size(&(meshes->array_), scene->mNumMeshes);
  g_array_set_clear_func(&(meshes->array_), (GDestroyNotify) _mesh_clear0);

  /* map buffers */
  __gl_try_catch(
    vertices = (DsModelVertex*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    indices = (DsModelIndex*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Load onto buffers
 *
 */

  for(i = 0, _v = 0, _i = 0;
      i < scene->mNumMeshes;
      i++)
  {
    mesh = scene->mMeshes[i];
    meshes->a[i].base_vertex = _v;
    meshes->a[i].index_offset = _i;
    meshes->a[i].indices = 0;

    /* copy vertices */
    for(j = 0;
        j < mesh->mNumVertices;
        j++)
    {
      copy_vertex(self, mesh, j, &(vertices[_v++]));
    }

    /* copy indices */
    for(j = 0;
        j < mesh->mNumFaces;
        j++)
    {
      face = &(mesh->mFaces[j]);
      G_STATIC_ASSERT(sizeof(face->mIndices[0]) == sizeof(DsModelIndex));

#if HAVE_MEMCPY
      memcpy(&(indices[_i]), face->mIndices, sizeof(DsModelIndex) * face->mNumIndices);
#else
      for(k = 0;
          k < face->mNumIndices;
          k++)
      {
        indices[_i + k] = face->mIndices[k];
      }
#endif // HAVE_MEMCPY
      meshes->a[i].indices += face->mNumIndices;
      _i += face->mNumIndices;
    }

    /* load textures */
    guint tid = mesh->mMaterialIndex;
    if G_UNLIKELY
      (tios->a[tid].tex == NULL)
    {
      tios->a[tid].tex =
      load_texture_file(self, scene->mMaterials[tid], cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }

    tios->a[tid].meshes =
    (union _DsModelMeshList*)
    g_list_append
    (&(tios->a[tid].meshes->list_),
     &(meshes->a[i]));
  }

/*
 * Finish
 *
 */

  self->priv->tios = ds_array_ref(tios);
  self->priv->meshes = ds_array_ref(meshes);
  self->vbo = ds_steal_handle_id(&vbo);
  self->ibo = ds_steal_handle_id(&ibo);

_error_:
  __gl_try_catch(
    if G_UNLIKELY(indices != NULL)
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    if G_UNLIKELY(vertices != NULL)
      glUnmapBuffer(GL_ARRAY_BUFFER);
    if G_UNLIKELY(ibo != 0)
      glDeleteBuffers(1, &ibo);
    if G_UNLIKELY(vbo != 0)
      glDeleteBuffers(1, &vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  ,
    g_warning
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  );

  ds_array_unref(tios);
  ds_array_unref(meshes);

  _ds_model_import_free(self, scene);
  g_clear_object(&(self->priv->source));
  g_clear_pointer(&(self->priv->filename), g_free);
return success;
}

static gboolean
ds_model_g_initable_iface_init_sync(GInitable    *pself,
                                    GCancellable *cancellable,
                                    GError      **error)
{
  DsModel* self = (DsModel*) pself;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_assert(self->priv->source);

/*
 * Class vertex array lazy load
 *
 */

  success =
  _ds_model_make_vao(self, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Load object
 *
 */

  success =
  load_object_file(self, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
return success;
}

static void
ds_model_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_model_g_initable_iface_init_sync;
}

static void
ds_model_ds_mvp_holder_iface_notify_model(DsMvpHolder* pself)
{
  DS_MODEL(pself)->priv->notified = TRUE;
}

static void
update_model(DsModelPrivate* priv)
{
  glm_mat4_identity(priv->model);
  glm_translate(priv->model, priv->position);
  glm_scale(priv->model, priv->scale);
  priv->notified = TRUE;
}

static void
ds_model_ds_mvp_holder_iface_set_position(DsMvpHolder* pself, gfloat* src)
{
  DsModelPrivate* priv = ((DsModel*)pself)->priv;
  glm_vec3_copy(src, priv->position);
  update_model(priv);
}

static void
ds_model_ds_mvp_holder_iface_get_position(DsMvpHolder* pself, gfloat* dst)
{
  DsModelPrivate* priv = ((DsModel*) pself)->priv;
  glm_vec3_copy(priv->position, dst);
}

static void
ds_model_ds_mvp_holder_iface_set_scale(DsMvpHolder* pself, gfloat* src)
{
  DsModelPrivate* priv = ((DsModel*) pself)->priv;
  glm_vec3_copy(src, priv->scale);
  update_model(priv);
}

static void
ds_model_ds_mvp_holder_iface_get_scale(DsMvpHolder* pself, gfloat* dst)
{
  DsModelPrivate* priv = ((DsModel*) pself)->priv;
  glm_vec3_copy(priv->scale, dst);
}

static void
ds_model_ds_mvp_holder_iface_init(DsMvpHolderIface* iface)
{
  iface->p_model = G_PRIVATE_OFFSET(DsModel, model);
  iface->notify_model = ds_model_ds_mvp_holder_iface_notify_model;
  iface->set_position = ds_model_ds_mvp_holder_iface_set_position;
  iface->get_position = ds_model_ds_mvp_holder_iface_get_position;
  iface->set_scale = ds_model_ds_mvp_holder_iface_set_scale;
  iface->get_scale = ds_model_ds_mvp_holder_iface_get_scale;
}

static gboolean
ds_model_ds_renderable_iface_compile(DsRenderable* pself, DsRenderState* state, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsModelClass* klass =
  DS_MODEL_GET_CLASS(pself);
  DsModelPrivate* priv =
  ((DsModel*) pself)->priv;

  GLuint program;
  GLuint uloc;
  guint i;

  program = ds_render_state_get_current_program(state);

/*
 * VAO switching
 *
 */

  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexArray),
   1,
   (guintptr) klass->vao);

/*
 * Texture binding
 *
 */

  for(i = 0;
      i < G_N_ELEMENTS(tex_uniforms);
      i++)
  {
    __gl_try_catch(
      uloc = glGetUniformLocation(program, tex_uniforms[i]);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    if G_UNLIKELY(uloc == (-1))
      continue;

    __gl_try_catch(
      glUniform1i(uloc, i);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

/*
 * Update model matrix
 *
 */

  __gl_try_catch(
    uloc = glGetUniformLocation(program, A_MVP);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_LIKELY(uloc != (-1))
  {
    JitState* ctx =
    (JitState*) state;

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(_ds_jit_helper_update_model),
     FALSE,
     2,
     (guintptr) &(ctx->mvps),
     (guintptr) priv->model);

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(_ds_jit_helper_update_mvp),
     FALSE,
     1,
     (guintptr) &(ctx->mvps));

    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(glUniformMatrix4fv),
     TRUE,
     4,
     (guintptr) uloc,
     (guintptr) 1,
     (guintptr) GL_FALSE,
     (guintptr) &(ctx->mvps.mvp));
  }

/*
 * Chain-up implementations
 *
 */

  success =
  klass->compile((DsModel*) pself, state, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
return success;
}

static void
ds_model_ds_renderable_iface_init(DsRenderableIface* iface)
{
  iface->compile = ds_model_ds_renderable_iface_compile;
}

static void
ds_model_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsModel* self = DS_MODEL(pself);
  switch(prop_id)
  {
  case prop_source:
    g_set_object(&(self->priv->source), g_value_get_object(value));
    break;
  case prop_name:
    g_clear_pointer(&(self->priv->filename), g_free);
    self->priv->filename = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_model_base_class_init(DsModelClass* klass)
{
  klass->vao = 0;
}

static void
ds_model_base_class_fini(DsModelClass* klass)
{
  GLuint* pvao = &(klass->vao);
  if G_LIKELY((*pvao) != 0)
  {
    __gl_try_catch(
      glDeleteVertexArrays(1, pvao);
      *pvao = 0;
    ,
      g_warning
      ("(%s: %i): %s: %i: %s\r\n",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string(glerror->domain),
       glerror->code,
       glerror->message);
      g_error_free(glerror);
    );
  }
}

static gboolean
ds_model_class_compile(DsModel* self, DsRenderState* state, GCancellable* cancellable, GError** error)
{
  g_warning
  ("DsModel::compile not implemented for '%s'\r\n",
   g_type_name(G_TYPE_FROM_INSTANCE(self)));
return FALSE;
}

static void
ds_model_class_finalize(GObject* pself)
{
  DsModel* self = DS_MODEL(pself);
  ds_array_unref(self->priv->tios);
  ds_array_unref(self->priv->meshes);
  g_clear_pointer(&(self->priv->filename), g_free);

  __gl_try_catch(
    glDeleteBuffers(2, self->bos);
  ,
    g_warning
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  );

G_OBJECT_CLASS(ds_model_parent_class)->finalize(pself);
}

static void
ds_model_class_dispose(GObject* pself) {
  DsModel* self = DS_MODEL(pself);
  g_clear_object(&(self->priv->source));
G_OBJECT_CLASS(ds_model_parent_class)->dispose(pself);
}

static void
ds_model_class_init(DsModelClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  klass->compile = ds_model_class_compile;

  oclass->set_property = ds_model_class_set_property;
  oclass->finalize = ds_model_class_finalize;
  oclass->dispose = ds_model_class_dispose;

  properties[prop_source] =
    g_param_spec_object
    ("source",
     "source",
     "source",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_name] =
    g_param_spec_string
    ("name",
     "name",
     "name",
     NULL,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static void
ds_model_class_fini(DsModelClass* klass)
{
}

static void
ds_model_init(DsModel* self)
{
  self->priv = G_STRUCT_MEMBER_P(self, DsModel_private_offset);
  self->priv->notified = TRUE;

  glm_mat4_identity(self->priv->model);
}

/*
 * Object methods
 *
 */

G_GNUC_INTERNAL
void
_ds_model_iterate_tio_groups(DsModel* self,
                             DsModelTioIterator foreach_tio,
                             gpointer user_data)
{
  DsModelTioArray* tios = self->priv->tios;
  union _DsModelMeshList* meshes = NULL;
  gboolean continue_;
  guint i;

  for(i = 0;
      i < tios->len;
      i++)
  if G_LIKELY(tios->a[i].meshes != NULL)
  {
    continue_ =
    foreach_tio(self, tios->a[i].tex, (GList*) tios->a[i].meshes, user_data);
    if(continue_ == G_SOURCE_REMOVE)
      return;
  }
}
