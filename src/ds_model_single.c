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
#include <ds_model_private.h>
#include <ds_renderable.h>

/**
 * SECTION:dsmodelsingle
 * @Short_description: Implementation of #DsModel
 * @Title: DsModelSingle
 *
 * DsModelSingle implements #DsModel for single-instanced
 * models.
 *
 */

#define DS_TYPE_MODEL_SINGLE            (ds_model_single_get_type ())
#define DS_MODEL_SINGLE(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_MODEL_SINGLE, DsModelSingle))
#define DS_MODEL_SINGLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_MODEL_SINGLE, DsModelSingleClass))
#define DS_IS_MODEL_SINGLE(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_MODEL_SINGLE))
#define DS_IS_MODEL_SINGLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_MODEL_SINGLE))
#define DS_MODEL_SINGLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_MODEL_SINGLE, DsModelSingleClass))

typedef struct _DsModelSingle       DsModelSingle;
typedef struct _DsModelSingleClass  DsModelSingleClass;

#define _g_array_unref0(var) ((var == NULL) ? NULL : (var = (g_array_unref (var), NULL)))
#define _ds_array_unref0(var) ((var == NULL) ? NULL : (var = (g_array_unref ((GArray*) var), NULL)))

#define TYPEOF_COUNT    GLsizei
#define TYPEOF_INDICES  GLvoid*
#define TYPEOF_BASES    GLint

/*
 * Object definition
 *
 */

struct _DsModelSingle
{
  DsModel parent_instance;

  union
  {
    GArray array_;
    struct
    {
      struct _Group
      {
        DsModelTexture* tex;
        GArray* counts;
        GArray* indices;
        GArray* bases;
      } *a;
      guint len;
    };
  } *groups;
};

struct _DsModelSingleClass
{
  DsModelClass parent_class;
};

G_DEFINE_TYPE_WITH_CODE
(DsModelSingle,
 ds_model_single,
 DS_TYPE_MODEL,
 );

static gboolean
foreach_tio(DsModel* pself, DsModelTexture* tex, GList* meshes)
{
  struct _Group group = {0};
  DsModelMesh* mesh = NULL;

  group.tex = tex;
  group.counts = g_array_new(0, 1, sizeof(TYPEOF_COUNT));
  group.indices = g_array_new(0, 1, sizeof(TYPEOF_INDICES));
  group.bases = g_array_new(0, 1, sizeof(TYPEOF_BASES));
  g_array_append_val(&(((DsModelSingle*) pself)->groups->array_), group);

  TYPEOF_COUNT count_;
  TYPEOF_INDICES index_;
  TYPEOF_BASES base_;

  for(;
      meshes != NULL;
      meshes = meshes->next)
  {
    mesh = meshes->data;
    count_ = mesh->indices;
    index_ = GUINT_TO_POINTER(mesh->index_offset);
    base_ = mesh->base_vertex;

    g_array_append_vals(group.counts, &count_, 1);
    g_array_append_vals(group.indices, &index_, 1);
    g_array_append_vals(group.bases, &base_, 1);
  }
return G_SOURCE_CONTINUE;
}

static gboolean
generate_groups(DsModelSingle* self)
{
  self->groups = (gpointer)
  g_array_new(0, 0, sizeof(struct _Group));

/*
 * Iterate through meshes
 *
 */

  _ds_model_iterate_tio_groups((DsModel*) self, (DsModelTioIterator) foreach_tio, NULL);
}

static gboolean
ds_model_single_class_compile(DsModel* pself, DsRenderState* state, GCancellable* cancellable, GError** error)
{
  DsModelSingle* self = DS_MODEL_SINGLE(pself);
  DsModel* model = DS_MODEL(self);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  struct _Group* groups;
  guint i;

  if G_UNLIKELY(self->groups == NULL)
  {
    generate_groups(self);
  }

/*
 * Draw meshes
 *
 */

#if GL_VERSION_3_2 == 0
# error "Minimun OpenGL version v3.2 is required"
#endif // GL_VERSION_3_2

  for(i = 0, groups = self->groups->a;
      i < self->groups->len;
      i++)
  {
    _ds_model_compile_switch_texture(groups[i].tex, state);

    ds_render_state_pcall
    (state,
     G_CALLBACK(glMultiDrawElementsBaseVertex),
     6,
     (guintptr) GL_TRIANGLES,
     (guintptr) groups[i].counts->data,
     (guintptr) GL_UNSIGNED_INT,
     (guintptr) groups[i].indices->data,
     (guintptr) groups[i].counts->len,
     (guintptr) groups[i].bases->data);
  }

_error_:
return success;
}

static void
ds_model_single_class_finalize(GObject* pself)
{
  DsModelSingle* self = DS_MODEL_SINGLE(pself);
  _ds_array_unref0(self->groups);
G_OBJECT_CLASS(ds_model_single_parent_class)->finalize(pself);
}

static void
ds_model_single_class_init(DsModelSingleClass* klass)
{
  DsModelClass* mclass = DS_MODEL_CLASS(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  mclass->compile = ds_model_single_class_compile;

  oclass->finalize = ds_model_single_class_finalize;
}

static void
group_free(struct _Group* group)
{
  _g_array_unref0(group->counts);
  _g_array_unref0(group->indices);
  _g_array_unref0(group->bases);
}

static void
ds_model_single_init(DsModelSingle* self)
{
}

/*
 * Object methods
 *
 */

/**
 * ds_model_single_new: (constructor)
 * @source: source directory where model and all it data resides.
 * @name: main model filename (note that must be relative to @source).
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Create a new instance of #DsModelSingle object.
 *
 * Returns: (transfer full): a #DsModel derived instance.
 */
DsModel*
ds_model_single_new(GFile* source, const gchar* name, GCancellable* cancellable, GError** error)
{
  return (DsModel*)
  g_initable_new
  (DS_TYPE_MODEL_SINGLE,
   cancellable,
   error,
   "source", source,
   "name", name,
   NULL);
}
