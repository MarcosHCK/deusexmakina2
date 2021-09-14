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
#include <ds_pipeline_private.h>

G_DEFINE_QUARK(ds-pipeline-error-quark,
               ds_pipeline_error);

static
void ds_pipeline_g_initable_iface_init(GInitableIface* iface);

typedef struct _DsPipelineEntry       DsPipelineEntry;
typedef union  _DsPipelineObjectList  DsPipelineObjectList;
typedef struct _DsPipelineObjectEntry DsPipelineObjectEntry;

GLuint
_ds_shader_get_pid(DsShader *shader);

DS_DEFINE_SNIPPET(function_prologue);
DS_DEFINE_SNIPPET(function_epilogue);
DS_DEFINE_SNIPPET(breakpoint);

/*
 * Object definition
 *
 */

struct _DsPipeline
{
  GObject parent_instance;

  /*<private>*/
  GHashTable* shaders;
  GMemoryOutputStream* binary;
  GDataOutputStream* code;
  guint modified : 1;
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
      GList* next;
      GList* prev;
    } c;
  } *objects;
};

G_DEFINE_TYPE_WITH_CODE
(DsPipeline,
 ds_pipeline,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_pipeline_g_initable_iface_init));

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

static
void ds_pipeline_class_finalize(GObject* pself) {
  DsPipeline* self = DS_PIPELINE(pself);
  g_hash_table_unref(self->shaders);
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
  self->binary = (GMemoryOutputStream*)
  g_memory_output_stream_new_resizable();
  g_assert(g_seekable_can_seek(G_SEEKABLE(self->binary)));
  self->code = (GDataOutputStream*)
  g_data_output_stream_new(G_OUTPUT_STREAM(self->binary));
  g_data_output_stream_set_byte_order(self->code, G_DATA_STREAM_BYTE_ORDER_HOST_ENDIAN);
  self->modified = TRUE;
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
                          DsShader     *object)
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
                          DsShader     *object)
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
  GDataOutputStream* code = pipeline->code;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  /* reset binary stream */
  g_seekable_seek
  (G_SEEKABLE(pipeline->binary),
   0,
   G_SEEK_SET,
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  /* copy prologue */
  g_output_stream_write
  (G_OUTPUT_STREAM(code),
   DS_SNIPPET_POINTER(function_prologue),
   DS_SNIPPET_SIZE(function_prologue),
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  /* copy epilogue */
  g_output_stream_write
  (G_OUTPUT_STREAM(code),
   DS_SNIPPET_POINTER(function_epilogue),
   DS_SNIPPET_SIZE(function_epilogue),
   cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  if G_LIKELY(success == TRUE)
    pipeline->modified = FALSE;
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

  gpointer code =
  g_memory_output_stream_get_data(pipeline->binary);
  asm volatile("call *%0"::"r"(code):);
}
