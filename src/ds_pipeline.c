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
#include <ds_callable.h>
#include <ds_macros.h>
#include <ds_pipeline.h>
#include <GL/glew.h>
#include <jit/jit.h>

G_DEFINE_QUARK(ds-pipeline-error-quark,
               ds_pipeline_error);

static
void ds_pipeline_g_initable_iface_init(GInitableIface* iface);
static
void ds_pipeline_ds_callable_iface_init(DsCallableIface* iface);

typedef struct _DsPipelineEntry       DsPipelineEntry;
typedef union  _DsPipelineObjectList  DsPipelineObjectList;
typedef struct _DsPipelineObjectEntry DsPipelineObjectEntry;

GLuint
_ds_shader_get_pid(DsShader *shader);

/*
 * Object definition
 *
 */

struct _DsPipeline
{
  GObject parent_instance;

  /*<private>*/
  GHashTable* shaders;
  guint modified : 1;
  JitState ctx;
  GFunc code;
};

struct _DsPipelineEntry
{
  DsShader* shader;
  union _DsPipelineObjectList
  {
    GList list_;
    struct _DsPipelineObjectEntry
    {
      DsRenderable* object;
      DsPipelineObjectList* next;
      DsPipelineObjectList* prev;
    } c;
  } *objects;
};

G_DEFINE_TYPE_WITH_CODE
(DsPipeline,
 ds_pipeline,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_pipeline_g_initable_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_CALLABLE,
  ds_pipeline_ds_callable_iface_init));

static gboolean
ds_pipeline_g_initable_iface_init_sync(GInitable     *pself,
                                       GCancellable  *cancellable,
                                       GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
_error_:
return success;
}

static
void ds_pipeline_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_pipeline_g_initable_iface_init_sync;
}

static DsPipeline*
_callable_new(gpointer null_, GCancellable* cancellable, GError** error)
{
  return ds_pipeline_new(cancellable, error);
}

static
void ds_pipeline_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new),
   ds_cclosure_marshal_OBJECT__OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__OBJECT_POINTERv,
   DS_TYPE_PIPELINE,
   2,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);

  ds_callable_iface_add_method
  (iface,
   "register_shader",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(ds_pipeline_register_shader),
   ds_cclosure_marshal_VOID__STRING_OBJECT,
   ds_cclosure_marshal_VOID__STRING_OBJECTv,
   G_TYPE_NONE,
   2,
   G_TYPE_STRING,
   DS_TYPE_SHADER);

  ds_callable_iface_add_method
  (iface,
   "unregister_shader",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(ds_pipeline_unregister_shader),
   g_cclosure_marshal_VOID__STRING,
   g_cclosure_marshal_VOID__STRINGv,
   G_TYPE_NONE,
   1,
   G_TYPE_STRING);

  ds_callable_iface_add_method
  (iface,
   "append_object",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(ds_pipeline_append_object),
   ds_cclosure_marshal_VOID__STRING_OBJECT,
   ds_cclosure_marshal_VOID__STRING_OBJECTv,
   G_TYPE_NONE,
   2,
   G_TYPE_STRING,
   DS_TYPE_RENDERABLE);

  ds_callable_iface_add_method
  (iface,
   "remove_object",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(ds_pipeline_remove_object),
   ds_cclosure_marshal_VOID__STRING_OBJECT,
   ds_cclosure_marshal_VOID__STRING_OBJECTv,
   G_TYPE_NONE,
   2,
   G_TYPE_STRING,
   DS_TYPE_RENDERABLE);

  ds_callable_iface_add_method
  (iface,
   "update",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(ds_pipeline_update),
   ds_cclosure_marshal_BOOLEAN__OBJECT_POINTER,
   ds_cclosure_marshal_BOOLEAN__OBJECT_POINTERv,
   G_TYPE_BOOLEAN,
   2,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static
void ds_pipeline_class_finalize(GObject* pself) {
  DsPipeline* self = DS_PIPELINE(pself);
  g_hash_table_unref(self->shaders);
  _ds_jit_compile_free(&(self->ctx));
G_OBJECT_CLASS(ds_pipeline_parent_class)->finalize(pself);
}

static
void ds_pipeline_class_dispose(GObject* pself) {
  DsPipeline* self = DS_PIPELINE(pself);
  g_hash_table_remove_all(self->shaders);
G_OBJECT_CLASS(ds_pipeline_parent_class)->dispose(pself);
}

static
void ds_pipeline_class_init(DsPipelineClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  oclass->finalize = ds_pipeline_class_finalize;
  oclass->dispose = ds_pipeline_class_dispose;
}

static void
_g_object_unref0(gpointer var)
{
  (var == NULL) ? NULL : (var = (g_object_unref (var), NULL));
}

static void
ds_entry_free(DsPipelineEntry* entry) {
  g_clear_object(&(entry->shader));
  g_list_free_full
  (&(entry->objects->list_),
   (GDestroyNotify)
   _g_object_unref0);
  entry->objects = NULL;
  g_slice_free
  (DsPipelineEntry,
   entry);
}

static void
_ds_entry_free0(gpointer var)
{
  (var == NULL) ? NULL : (var = (ds_entry_free (var), NULL));
}

static
void ds_pipeline_init(DsPipeline* self) {
  self->shaders = g_hash_table_new_full
  ((GHashFunc) g_str_hash,
   (GEqualFunc) g_str_equal,
   (GDestroyNotify) g_free,
   (GDestroyNotify) _ds_entry_free0);
}

/*
 * Object methods
 *
 */

DsPipeline*
ds_pipeline_new(GCancellable   *cancellable,
                GError        **error)
{
  return
  g_initable_new
  (DS_TYPE_PIPELINE,
   cancellable,
   error,
   NULL);
}

void
ds_pipeline_register_shader(DsPipeline   *pipeline,
                            const gchar  *shader_name,
                            DsShader     *shader)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_SHADER(shader));

  gboolean has =
  g_hash_table_lookup_extended
  (pipeline->shaders,
   shader_name,
   NULL, NULL);

  if G_UNLIKELY(has == TRUE)
  {
    g_warning("Attempt to register a shader twice\r\n");
    return;
  }

  DsPipelineEntry* entry =
  g_slice_new(DsPipelineEntry);
  entry->shader = g_object_ref(shader);
  entry->objects = NULL;

  g_hash_table_insert
  (pipeline->shaders,
   g_strdup(shader_name),
   entry);

  pipeline->modified = TRUE;
}

void
ds_pipeline_unregister_shader(DsPipeline   *pipeline,
                              const gchar  *shader_name)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);

  gboolean has =
  g_hash_table_lookup_extended
  (pipeline->shaders,
   shader_name,
   NULL, NULL);

  if G_UNLIKELY(has != TRUE)
  {
    g_warning("Attempt to remove an inexistent shader from pipeline\r\n");
    return;
  }
  else
  {
    g_hash_table_remove
    (pipeline->shaders,
     shader_name);
  }

  pipeline->modified = TRUE;
}

void
ds_pipeline_append_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          DsRenderable *object)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_RENDERABLE(object));
  DsPipelineEntry* entry = NULL;

  gboolean has =
  g_hash_table_lookup_extended
  (pipeline->shaders,
   shader_name,
   NULL,
   (gpointer*)
   &entry);

  if G_UNLIKELY(has != TRUE)
  {
    g_warning("Attempt to append an object onto an inexistent shader\r\n");
    return;
  }

  entry->objects =
  (DsPipelineObjectList*)
  g_list_append
  (&(entry->objects->list_),
   g_object_ref(object));
  pipeline->modified = TRUE;
}

void
ds_pipeline_remove_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          DsRenderable *object)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_RENDERABLE(object));
  DsPipelineEntry* entry = NULL;

  gboolean has =
  g_hash_table_lookup_extended
  (pipeline->shaders,
   shader_name,
   NULL,
   (gpointer*)
   &entry);

  if G_UNLIKELY(has != TRUE)
  {
    g_warning("Attempt to remove an object onto an inexistent shader\r\n");
    return;
  }

  entry->objects =
  (DsPipelineObjectList*)
  g_list_remove
  (&(entry->objects->list_),
   g_object_ref(object));
  pipeline->modified = TRUE;
}

gboolean
ds_pipeline_update(DsPipeline    *pipeline,
                   GCancellable  *cancellable,
                   GError       **error)
{
  g_return_val_if_fail(DS_IS_PIPELINE(pipeline), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  JitState* ctx = &(pipeline->ctx);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  /* begin code */
  _ds_jit_compile_free(ctx);
  _ds_jit_compile_start(ctx);

  /* cycle through programs */
  GHashTableIter iter;
  DsPipelineEntry* entry;
  g_hash_table_iter_init(&iter, pipeline->shaders);
  while(g_hash_table_iter_next(&iter, NULL, (gpointer*) &entry))
  {
  /*
   * Use program
   *
   */
    GLuint pid =
    _ds_shader_get_pid(entry->shader);
    _ds_jit_compile_use_shader(ctx, pid);
  }

  /* finalize code */
  _ds_jit_compile_end(ctx);

_error_:
  if G_LIKELY(success == TRUE)
  {
    pipeline->code = jitmain;
    pipeline->modified = FALSE;
  }
return success;
}

void
ds_pipeline_execute(DsPipeline* pipeline)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));

  if G_UNLIKELY(pipeline->modified == TRUE)
  {
    g_warning("Attempt to execute a pipeline containing changes\r\n");

    GError* tmp_err = NULL;
    ds_pipeline_update(pipeline, NULL, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_critical
      ("%s: %i: %s\r\n",
       g_quark_to_string(tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free(tmp_err);
      g_assert_not_reached();
    }
  }

  GError* tmp_err = NULL;
  pipeline->code(&tmp_err, NULL);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("%s: %i: %s\r\n",
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    g_assert_not_reached();
  }
}
