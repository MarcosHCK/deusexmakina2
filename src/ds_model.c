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
#include <assimp/cimport.h>
#include <assimp/cfileio.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <cglm/cglm.h>
#include <ds_callable.h>
#include <ds_dds.h>
#include <ds_macros.h>
#include <ds_model.h>
#include <ds_mvpholder.h>
#include <ds_renderable.h>
#include <SDL.h>

G_DEFINE_QUARK(ds-model-error-quark,
               ds_model_error);

#define TEXTURE_DIRECT_PICK (0)

static
void ds_model_g_initable_iface_init(GInitableIface* iface);
static
void ds_model_ds_callable_iface_init(DsCallableIface* iface);
static
void ds_model_ds_mvp_holder_iface_init(DsMvpHolderIface* iface);
static
void ds_model_ds_renderable_iface_init(DsRenderableIface* iface);

typedef union  _MeshList      MeshList;
typedef struct _MeshEntry     MeshEntry;
typedef union  _VertexArray   VertexArray;
typedef struct _Vertex        Vertex;
typedef union  _IndexArray    IndexArray;
typedef unsigned int          Index;
typedef union  _TextureArray  TextureArray;
typedef struct _Texture       Texture;

/* keep sync with ds_gl.h */
const gint gl2ai[] = {
  aiTextureType_NONE,       // DS_GL_TEXTURE_TYPE_NONE
  aiTextureType_DIFFUSE,    // DS_GL_TEXTURE_TYPE_DIFFUSE
  aiTextureType_SPECULAR,   // DS_GL_TEXTURE_TYPE_SPECULAR
  aiTextureType_NORMALS,    // DS_GL_TEXTURE_TYPE_NORMALS
  aiTextureType_HEIGHT,     // DS_GL_TEXTURE_TYPE_HEIGHT
};

#define _texture_unref0(var) ((var == NULL) ? NULL : (var = (texture_unref (var), NULL)))
#define _aiReleaseImport0(var) ((var == NULL) ? NULL : (var = (aiReleaseImport (var), NULL)))

/*
 * Object definition
 *
 */

struct _DsModel
{
  GObject parent_instance;

  /*<private>*/
  GFile* source;
  gchar* name;
  GHashTable* textures;
  union _MeshList
  {
    GList list_;
    struct
    {
      struct _MeshEntry
      {
        VertexArray* vertexes;
        IndexArray* indices;
        TextureArray* textures;
      } *d;

      MeshList* next;
      MeshList* prev;
    };
  } *meshes;

  mat4 model;
};

union _VertexArray
{
  GArray array_;
  struct
  {
    struct _Vertex
    {
      vec3 position;
      vec3 normal;
      union
      {
        vec3 uvw;
        vec2 uv;
      };
      vec3 tangent;
      vec3 bitangent;
    } *a;

    guint len;
  };
};

union _IndexArray
{
  GArray array_;
  struct
  {
    Index* a;
    guint len;
  };
};

union _TextureArray
{
  GPtrArray array_;
  struct
  {
    Texture* a;
    guint len;
  };
};

enum {
  prop_0,
  prop_source,
  prop_name,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsModel,
 ds_model,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_model_g_initable_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_CALLABLE,
  ds_model_ds_callable_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_MVP_HOLDER,
  ds_model_ds_mvp_holder_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_RENDERABLE,
  ds_model_ds_renderable_iface_init));

struct _Texture
{
  grefcount refs;
  DsGLTextureType type;
  GLuint tio;
};

static Texture*
texture_new()
{
  Texture* texture =
  g_slice_new(Texture);
  g_ref_count_init(&(texture->refs));
  glGenTextures(1, &(texture->tio));
  texture->type = DS_GL_TEXTURE_TYPE_NONE;
return texture;
}

static Texture*
texture_ref(Texture* texture)
{
  g_ref_count_inc(&(texture->refs));
return texture;
}

static void
texture_unref(Texture* texture)
{
  gboolean zero =
  g_ref_count_dec(&(texture->refs));
  if(zero == TRUE)
  {
    glDeleteTextures(1, &(texture->tio));
    g_slice_free(Texture, texture);
  }
}

G_DEFINE_BOXED_TYPE
(Texture,
 texture,
 texture_ref,
 texture_unref);

static void
_mesh_entry_free0(MeshEntry* entry)
{
  if G_LIKELY(entry != NULL)
  {
    g_array_unref(&(entry->vertexes->array_));
    g_array_unref(&(entry->indices->array_));
    g_ptr_array_unref(&(entry->textures->array_));
    g_slice_free(MeshEntry, entry);
  }
}

typedef struct _FioData FioData;

struct _FioData
{
  FioData* chain;

  union
  {
    GFile* source;
    GInputStream* stream;
    GSeekable* seekable;
    GObject* object;
  };

  GCancellable* cancellable;
  GError* error;
};

static void
push_error0(FioData* data, GError* src)
{
  if G_UNLIKELY
    (data->error != NULL)
  {
    GError* dst = data->error;

    g_critical
    ("Setting a new error over a previous one\r\n"
     "Previous was %s: %i: %s\r\n",
     "New is %s: %i: %s\r\n",
     g_quark_to_string(dst->domain),
     dst->code,
     dst->message,
     g_quark_to_string(src->domain),
     src->code,
     src->message);
    g_error_free(dst);
    data->error = src;
  }
  else
  {
    data->error = src;
  }
}

static void
push_error(FioData* data, GError* src)
{
  g_return_if_fail(src != NULL);
  if(data->chain != NULL)
    push_error0(data->chain, src);
  else
    push_error0(data, src);
}

static GError*
pop_error(FioData* data)
{
  if(data->chain != NULL)
    return g_steal_pointer(&(data->chain->error));
  else
    return g_steal_pointer(&(data->error));
return NULL;
}

static gboolean
scene_import_texture(DsModel                   *self,
                     MeshEntry                 *entry,
                     const C_STRUCT aiScene    *scene,
                     const C_STRUCT aiMaterial *material,
                     DsGLTextureType            type,
                     GCancellable              *cancellable,
                     GError                   **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  C_ENUM aiTextureType aitype = gl2ai[type];
  guint i, count = aiGetMaterialTextureCount(material, aitype);
  TextureArray* textures = entry->textures;
  Texture* texture = NULL;
  GFile* file = NULL;

  for(i = 0;
      i < count;
      i++)
  {
    C_STRUCT aiString name;
    memset(&name, 0, sizeof(name));

    aiGetMaterialTexture(material, aitype, i, &name, NULL, NULL, NULL, NULL, NULL, NULL);
    g_assert(name.length > 0);

    gboolean has =
    g_hash_table_lookup_extended
    (self->textures,
     name.data,
     NULL,
     (gpointer*)
     &texture);
    if G_LIKELY(has == TRUE)
    {
      g_ptr_array_add
      (&(textures->array_),
       texture_ref(texture));
      texture = NULL;
    }
    else
    {
      /*
       * Load texture
       *
       */
      texture =
      texture_new();

      /* Bind texture object */
      __gl_try_catch(
        glBindTexture(GL_TEXTURE_2D, texture->tio);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      ,
        g_propagate_error(error, glerror);
        goto_error();
      );

      /* create file */
      _g_object_unref0(file);
      file = g_file_get_child(self->source, name.data);

      success =
      ds_dds_load_image(file, GL_TEXTURE_2D, cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }

      __gl_try_catch(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
      ,
        g_propagate_error(error, glerror);
        goto_error()
      );

      g_hash_table_insert(self->textures, g_strdup(name.data), texture);
      g_ptr_array_add(&(textures->array_), texture_ref(texture));
      texture = NULL;
    }
  }

_error_:
  _texture_unref0(texture);
  _g_object_unref0(file);

  __gl_try_catch(
    glBindTexture(GL_TEXTURE_2D, 0);
  ,
    g_warning
    ("(%s:%i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  );
return success;
}

static gboolean
scene_import_mesh(DsModel                *self,
                  const C_STRUCT aiScene *scene,
                  const C_STRUCT aiMesh  *mesh,
                  GCancellable           *cancellable,
                  GError                **error)
{
  MeshEntry* entry = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  guint i, j;

  VertexArray* vertexes = NULL;
  IndexArray* indices = NULL;
  TextureArray* textures = NULL;

/*
 * Prepare arrays
 *
 */

  entry =
  g_slice_new0(MeshEntry);
  vertexes =
  entry->vertexes = (VertexArray*)
  g_array_new(0, 1, sizeof(Vertex));
  indices =
  entry->indices = (IndexArray*)
  g_array_new(0, 1, sizeof(Index));
  textures =
  entry->textures = (TextureArray*)
  g_ptr_array_new_full
  (0,
   (GDestroyNotify)
   texture_unref);

/*
 * Load vertices
 *
 */

  g_array_set_size
  (&(vertexes->array_),
   mesh->mNumVertices);

  for(i = 0;
      i < mesh->mNumVertices;
      i++)
  {
    Vertex* v = &(vertexes->a[i]);

    /* position */
    v->position[0] = mesh->mVertices[i].x;
    v->position[1] = mesh->mVertices[i].y;
    v->position[2] = mesh->mVertices[i].z;

    /* normal */
    if G_LIKELY(mesh->mNormals != NULL)
    {
      v->normal[0] = mesh->mNormals[i].x;
      v->normal[1] = mesh->mNormals[i].y;
      v->normal[2] = mesh->mNormals[i].z;
    }

    /* texture */
    if G_LIKELY(mesh->mTextureCoords != NULL)
    {
      /* uv */
      switch(mesh->mNumUVComponents[0])
      {
      case 3:
        v->uvw[2] = mesh->mTextureCoords[0][i].z;
      case 2:
        v->uvw[1] = mesh->mTextureCoords[0][i].y;
      case 1:
        v->uvw[0] = mesh->mTextureCoords[0][i].x;
      case 0:
        break;
      default:
        g_critical
        ("mesh->mNumUVComponents[i] = %u\r\n",
         mesh->mNumUVComponents[i]);
        g_assert_not_reached();
        break;
      }

      /* tangent */
      if G_LIKELY(v->tangent != NULL)
      {
        v->tangent[0] = mesh->mTangents[i].x;
        v->tangent[1] = mesh->mTangents[i].y;
        v->tangent[2] = mesh->mTangents[i].z;
      }

      /* bitangent */
      if G_LIKELY(v->bitangent != NULL)
      {
        v->bitangent[0] = mesh->mBitangents[i].x;
        v->bitangent[1] = mesh->mBitangents[i].y;
        v->bitangent[2] = mesh->mBitangents[i].z;
      }
    }
  }

/*
 * Load indices
 *
 */

  for(i = 0;
      i < mesh->mNumFaces;
      i++)
  {
    g_array_append_vals
    (&(indices->array_),
     mesh->mFaces[i].mIndices,
     mesh->mFaces[i].mNumIndices);
  }

/*
 * Load textures
 *
 */

  C_STRUCT aiMaterial* material =
  scene->mMaterials[mesh->mMaterialIndex];

#if TEXTURE_DIRECT_PICK
# define scene_import_texture(type) \
  G_STMT_START { \
    success = \
    scene_import_texture(self, entry, scene, material, (type), cancellable, &tmp_err); \
    if G_UNLIKELY(tmp_err != NULL) \
    { \
      g_propagate_error(error, tmp_err); \
      goto_error(); \
    } \
  } G_STMT_END

  scene_import_texture(TEXTURE_TYPE_DIFFUSE);
  scene_import_texture(TEXTURE_TYPE_SPECULAR);
  scene_import_texture(TEXTURE_TYPE_NORMAL);
  scene_import_texture(TEXTURE_TYPE_HEIGHT);

# undef scene_import_texture
#else // TEXTURE_DIRECT_PICK
  GType etype =
  ds_gl_texture_type_get_type();
  GEnumClass* klass =
  g_type_class_ref(etype);
  GEnumValue* values = klass->values;

  for(i = 0;
      i < klass->n_values;
      i++)
  {
    success =
    scene_import_texture(self, entry, scene, material, values[i].value, cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      g_type_class_unref(klass);
      goto_error();
    }
  }

  g_type_class_unref(klass);
#endif // TEXTURE_DIRECT_PICK

/*
 * Finish by appending mesh data
 *
 */

  self->meshes = (MeshList*)
  g_list_append
  (&(self->meshes->list_),
   g_steal_pointer(&entry));

_error_:
  _mesh_entry_free0(entry);
return success;
}

static gboolean
scene_import_node(DsModel                *self,
                  const C_STRUCT aiScene *scene,
                  const C_STRUCT aiNode  *node,
                  GCancellable           *cancellable,
                  GError                **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  guint i;

  for(i = 0;
      i < node->mNumMeshes;
      i++)
  {
    C_STRUCT aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

    success =
    scene_import_mesh(self, scene, mesh, cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

  for(i = 0;
      i < node->mNumChildren;
      i++)
  {
    success =
    scene_import_node(self, scene, node->mChildren[i], cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

_error_:
return success;
}

static void
_fio_data_free0(FioData* data)
{
  if G_LIKELY(data != NULL)
  {
    g_clear_object(&(data->object));
    g_clear_object(&(data->cancellable));

    if G_UNLIKELY(data->error != NULL)
    {
      GError* tmp_err = data->error;

      g_critical
      ("Uncaught error: %s: %i: %s\r\n",
       g_quark_to_string(tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
    }

    g_slice_free(FioData, data);
  }
}

static size_t
ai_read_proc(C_STRUCT aiFile* ifile, char* buffer, size_t size, size_t count)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  gsize read = -1;

  g_input_stream_read_all
  (data->stream,
   buffer,
   (gsize)
   (size * count),
   &read,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

_error_:
  if G_UNLIKELY(success == FALSE)
    return (size_t) (-1);
return (size_t) read;
}

static size_t
ai_tell_proc(C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) ifile->UserData;
return (size_t) g_seekable_tell(data->seekable);
}

static size_t
ai_file_size_proc(C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GFileInfo* info = NULL;
  gsize size = -1;

  info =
  g_file_input_stream_query_info
  (G_FILE_INPUT_STREAM(data->stream),
   G_FILE_ATTRIBUTE_STANDARD_SIZE,
   data->cancellable,
   &tmp_err);

  size =
  g_file_info_get_attribute_uint64
  (info,
   G_FILE_ATTRIBUTE_STANDARD_SIZE);

_error_:
  _g_object_unref0(info);
return (size_t) size;
}

static C_ENUM aiReturn
ai_seek_proc(C_STRUCT aiFile* ifile, size_t offset, C_ENUM aiOrigin from)
{
  FioData* data = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GSeekType type = 0;

  switch(from)
  {
  case aiOrigin_SET:
    type = G_SEEK_SET;
    break;
  case aiOrigin_CUR:
    type = G_SEEK_CUR;
    break;
  case aiOrigin_END:
    type = G_SEEK_END;
    break;
  default:
    g_assert_not_reached();
    break;
  }

  g_seekable_seek
  (data->seekable,
   offset,
   type,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

_error_:
return success ? aiReturn_SUCCESS : aiReturn_FAILURE;
}

static C_STRUCT aiFile*
ai_open_proc(C_STRUCT aiFileIO* fio, const gchar* path, const gchar* mode)
{
  FioData* data = (FioData*) fio->UserData;
  C_STRUCT aiFile* ifile = NULL;
  GInputStream* stream = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_assert(mode[0] == 'r');

/*
 * Open file
 *
 */

  GFile* file =
  g_file_get_child(data->source, path);

  stream = (GInputStream*)
  g_file_read(file, data->cancellable, &tmp_err);
  g_object_unref(file);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data, tmp_err);
    goto_error();
  }

/*
 * Prepare aiFile structure
 *
 */

  FioData* data2 = g_slice_new(FioData);
  ifile = g_slice_new(C_STRUCT aiFile);
  ifile->UserData = (aiUserData) data2;

  data2->chain = data;
  data2->stream = g_steal_pointer(&stream);
  data2->cancellable = _g_object_ref0(data->cancellable);
  data2->error = NULL;

  ifile->ReadProc = ai_read_proc;
  ifile->WriteProc = NULL;
  ifile->TellProc = ai_tell_proc;
  ifile->FileSizeProc = ai_file_size_proc;
  ifile->SeekProc = ai_seek_proc;
  ifile->FlushProc = NULL;

_error_:
  g_clear_object(&stream);
  if G_UNLIKELY(success == FALSE)
  {
    if(ifile != NULL)
    {
      if(ifile->UserData)
      {
        g_clear_pointer
        ((FioData**)
         &(ifile->UserData),
         _fio_data_free0);
      }

      g_slice_free
      (C_STRUCT aiFile,
       ifile);
      ifile = NULL;
    }
  }
return ifile;
}

void
aio_close_proc(C_STRUCT aiFileIO* fio, C_STRUCT aiFile* ifile)
{
  FioData* data = (FioData*) fio->UserData;
  FioData* data2 = (FioData*) ifile->UserData;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_input_stream_close
  (data2->stream,
   data2->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    push_error(data2, tmp_err);
  }
}

static gboolean
ds_model_g_initable_iface_init_sync(GInitable    *pself,
                                    GCancellable *cancellable,
                                    GError      **error)
{
  DsModel* self = (DsModel*) pself;
  const C_STRUCT aiScene* scene = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  FioData* data = NULL;

  C_STRUCT aiFileIO fio = {0};
  data = g_slice_new0(FioData);

  data->source = g_object_ref(self->source);
  data->cancellable = _g_object_ref0(cancellable);

  fio.OpenProc = ai_open_proc;
  fio.CloseProc = aio_close_proc;
  fio.UserData = (aiUserData) data;

  scene =
  aiImportFileEx
  (self->name,
   aiProcess_Triangulate
   | aiProcess_GenSmoothNormals
   | aiProcess_CalcTangentSpace,
   &fio);

  tmp_err = pop_error(data);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  if G_UNLIKELY
    (scene == NULL
     || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
     || (scene->mRootNode == NULL))
  {
    g_set_error_literal
    (error,
     DS_MODEL_ERROR,
     DS_MODEL_ERROR_INCOMPLETE_IMPORT,
     "Incomplete import\r\n");
    goto_error();
  }

  success =
  scene_import_node(self, scene, scene->mRootNode, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  g_clear_object(&(self->source));
  g_clear_pointer(&(self->name), g_free);
  _aiReleaseImport0(scene);
  _fio_data_free0(data);
return success;
}

static
void ds_model_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_model_g_initable_iface_init_sync;
}

static DsModel*
_callable_new(gpointer      null_,
              GFile        *source,
              const gchar  *name,
              GCancellable *cancellable,
              GError      **error)
{
  return
  ds_model_new
  (source,
   name,
   cancellable,
   error);
}

static DsModel*
_callable_new_simple(gpointer      null_,
                     const gchar  *source,
                     const gchar  *name,
                     GCancellable *cancellable,
                     GError      **error)
{
  GFile *source_ = NULL;
  source_ = g_file_new_for_path(source);

  DsModel* model =
  ds_model_new(source_, name, cancellable, error);

  g_object_unref(source_);
return model;
}

static
void ds_model_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new),
   ds_cclosure_marshal_OBJECT__OBJECT_STRING_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__OBJECT_STRING_OBJECT_POINTERv,
   DS_TYPE_MODEL,
   4,
   G_TYPE_FILE,
   G_TYPE_STRING,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);

  ds_callable_iface_add_method
  (iface,
   "new_simple",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new_simple),
   ds_cclosure_marshal_OBJECT__STRING_STRING_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__STRING_STRING_OBJECT_POINTERv,
   DS_TYPE_MODEL,
   4,
   G_TYPE_STRING,
   G_TYPE_STRING,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static
void ds_model_ds_mvp_holder_iface_set_position(DsMvpHolder* pself, gfloat* dst)
{
}

static
void ds_model_ds_mvp_holder_iface_get_position(DsMvpHolder* pself, gfloat* dst)
{
}

static
void ds_model_ds_mvp_holder_iface_init(DsMvpHolderIface* iface) {
  iface->p_model = G_STRUCT_OFFSET(DsModel, model);
  iface->set_position = ds_model_ds_mvp_holder_iface_set_position;
  iface->get_position = ds_model_ds_mvp_holder_iface_get_position;
}

static gboolean
ds_model_ds_renderable_iface_compile(DsRenderable        *pself,
                                     DsRenderState       *state,
                                     GLuint               program,
                                     GCancellable        *cancellable,
                                     GError             **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  g_assert_not_reached();
_error_:
return success;
}

static
void ds_model_ds_renderable_iface_init(DsRenderableIface* iface) {
  iface->compile = ds_model_ds_renderable_iface_compile;
}

static
void ds_model_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  DsModel* self = DS_MODEL(pself);
  switch(prop_id)
  {
  case prop_source:
    g_set_object(&(self->source), g_value_get_object(value));
    break;
  case prop_name:
    g_clear_pointer(&(self->name), g_free);
    self->name = g_value_dup_string(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_model_class_finalize(GObject* pself) {
  DsModel* self = DS_MODEL(pself);
  g_hash_table_remove_all(self->textures);
  g_hash_table_unref(self->textures);
  _g_free0(self->name);
G_OBJECT_CLASS(ds_model_parent_class)->finalize(pself);
}

static
void ds_model_class_dispose(GObject* pself) {
  DsModel* self = DS_MODEL(pself);
  g_clear_object(&(self->source));
G_OBJECT_CLASS(ds_model_parent_class)->dispose(pself);
}

static
void ds_model_class_init(DsModelClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */

  oclass->set_property = ds_model_class_set_property;
  oclass->finalize = ds_model_class_finalize;
  oclass->dispose = ds_model_class_dispose;

/*
 * Properties
 *
 */

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

static
void ds_model_init(DsModel* self) {
  self->textures =
  g_hash_table_new_full
  (g_str_hash,
   g_str_equal,
   (GDestroyNotify)
   g_free,
   (GDestroyNotify)
   texture_unref);
  self->meshes = NULL;
}

/*
 * Object methods
 *
 */

DsModel*
ds_model_new(GFile         *source,
             const gchar   *name,
             GCancellable  *cancellable,
             GError       **error)
{
  return (DsModel*)
  g_initable_new
  (DS_TYPE_MODEL,
   cancellable,
   error,
   "source", source,
   "name", name,
   NULL);
}
