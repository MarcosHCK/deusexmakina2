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
#include <cglm/cglm.h>
#include <ds_callable.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_pipeline.h>
#include <GL/glew.h>
#include <jit/jit.h>

G_DEFINE_QUARK(ds-pipeline-error-quark,
               ds_pipeline_error);
G_DEFINE_QUARK(ds-pipeline-priority-quark,
               ds_pipeline_priority);

static
void ds_pipeline_g_initable_iface_init(GInitableIface* iface);
static
void ds_pipeline_ds_callable_iface_init(DsCallableIface* iface);

typedef union  _ShaderList  ShaderList;
typedef struct _ShaderEntry ShaderEntry;
typedef union  _ObjectList  ObjectList;

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
  guint modified : 1;
  JitState ctx;
  JitMain main;

  /*<private>*/
  union _ShaderList
  {
    GList list_;
    struct
    {
      ShaderEntry* e;
      ShaderList* next;
      ShaderList* prev;
    };
  } *shaders;
};

struct _ShaderEntry
{
  DsShader* shader;
  guint n_objects;
  int priority;

  gchar* name;
  guint hash;

  union _ObjectList
  {
    GList list_;
    struct
    {
      DsRenderable* object;
      ObjectList* next;
      ObjectList* prev;
    };
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
   ds_cclosure_marshal_VOID__STRING_INT_OBJECT,
   ds_cclosure_marshal_VOID__STRING_INT_OBJECTv,
   G_TYPE_NONE,
   3,
   G_TYPE_STRING,
   G_TYPE_INT,
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
   ds_cclosure_marshal_VOID__STRING_INT_OBJECT,
   ds_cclosure_marshal_VOID__STRING_INT_OBJECTv,
   G_TYPE_NONE,
   3,
   G_TYPE_STRING,
   G_TYPE_INT,
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
  g_list_free(&(self->shaders->list_));
  _ds_jit_compile_free(&(self->ctx));
G_OBJECT_CLASS(ds_pipeline_parent_class)->finalize(pself);
}

static void
(_g_object_unref0)(gpointer var)
{
  (var == NULL) ? NULL : (var = (g_object_unref (var), NULL));
}

static void
shader_entry_free(ShaderEntry* entry)
{
  g_clear_object(&(entry->shader));
  g_clear_pointer(&(entry->name), g_free);

  g_list_free_full
  (&(entry->objects->list_),
   (GDestroyNotify)
   _g_object_unref0);
  entry->objects = NULL;

  g_slice_free(ShaderEntry, entry);
}

static void
_shader_entry_free0(gpointer var)
{
  (var == NULL) ? NULL : (var = (shader_entry_free (var), NULL));
}

static
void ds_pipeline_class_dispose(GObject* pself) {
  DsPipeline* self = DS_PIPELINE(pself);

  ShaderList* list;
  for(list = self->shaders;
      list != NULL;
      list = list->next)
  {
    g_clear_pointer
    (&(list->e),
     _shader_entry_free0);
  }

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

static
void ds_pipeline_init(DsPipeline* self) {
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

static ShaderEntry*
shader_list_has(ShaderList* shaders, const gchar* name)
{
  guint hash = g_str_hash(name);
  ShaderList* list;

  for(list = shaders;
      list != NULL;
      list = list->next)
  {
    if(hash == list->e->hash)
    {
      gboolean matches =
      g_str_equal(list->e->name, name);
      if(matches == TRUE)
      {
        return list->e;
      }
    }
  }
return NULL;
}

static gint
insert_sorted_shader(ShaderEntry *shader1,
                     ShaderEntry *shader2)
{
  int pr1 = shader1->priority;
  int pr2 = shader2->priority;
return (pr1 > pr2) ? 1 : -1;
}

void
ds_pipeline_register_shader(DsPipeline   *pipeline,
                            const gchar  *shader_name,
                            int           priority,
                            DsShader     *shader)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_SHADER(shader));

  gboolean has =
  shader_list_has(pipeline->shaders, shader_name) != NULL;
  if G_UNLIKELY(has == TRUE)
  {
    g_warning("Attempt to register a shader twice\r\n");
    return;
  }

  ShaderEntry* entry =
  g_slice_new(ShaderEntry);
  entry->shader = g_object_ref(shader);
  entry->n_objects = 0;
  entry->priority = priority;
  entry->objects = NULL;
  entry->name = g_strdup(shader_name);
  entry->hash = g_str_hash(shader_name);

  pipeline->shaders =
  (ShaderList*)
  g_list_insert_sorted
  (&(pipeline->shaders->list_),
   entry,
   (GCompareFunc)
   insert_sorted_shader);

  pipeline->modified = TRUE;
}

void
ds_pipeline_unregister_shader(DsPipeline   *pipeline,
                              const gchar  *shader_name)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);

  gpointer data =
  shader_list_has(pipeline->shaders, shader_name);
  if G_UNLIKELY(data == NULL)
  {
    g_warning("Attempt to remove an inexistent shader from pipeline\r\n");
    return;
  }
  else
  {
    pipeline->shaders =
    (ShaderList*)
    g_list_remove
    ((GList*)
     pipeline->shaders,
     data);
    _shader_entry_free0(data);
  }

  pipeline->modified = TRUE;
}

static gint
insert_sorted_object(DsRenderable  *object1,
                     DsRenderable  *object2)
{
  gpointer pp1 =
  g_object_get_qdata
  (G_OBJECT(object1),
   ds_pipeline_priority_quark());
  gpointer pp2 =
  g_object_get_qdata
  (G_OBJECT(object2),
   ds_pipeline_priority_quark());

  int pr1 = GPOINTER_TO_INT(pp1);
  int pr2 = GPOINTER_TO_INT(pp2);
return (pr1 > pr2) ? 1 : -1;
}

void
ds_pipeline_append_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          int           priority,
                          DsRenderable *object)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_RENDERABLE(object));
  ShaderEntry* entry = NULL;

  entry =
  shader_list_has(pipeline->shaders, shader_name);
  if G_UNLIKELY(entry == NULL)
  {
    g_warning("Attempt to append an object to an inexistent shader\r\n");
    return;
  }

  g_object_set_qdata
  (G_OBJECT(object),
   ds_pipeline_priority_quark(),
   GINT_TO_POINTER(priority));

  entry->objects =
  (ObjectList*)
  g_list_insert_sorted
  (&(entry->objects->list_),
   g_object_ref(object),
   (GCompareFunc)
   insert_sorted_object);
  pipeline->modified = TRUE;
  entry->n_objects++;
}

void
ds_pipeline_remove_object(DsPipeline   *pipeline,
                          const gchar  *shader_name,
                          DsRenderable *object)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  g_return_if_fail(shader_name != NULL);
  g_return_if_fail(DS_IS_RENDERABLE(object));
  ShaderEntry* entry = NULL;

  entry =
  shader_list_has(pipeline->shaders, shader_name);
  if G_UNLIKELY(entry == NULL)
  {
    g_warning("Attempt to remove an object from an inexistent shader\r\n");
    return;
  }

  entry->objects =
  (ObjectList*)
  g_list_remove
  (&(entry->objects->list_),
   g_object_ref(object));
  pipeline->modified = TRUE;
  entry->n_objects--;
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
  ShaderList* slist;
  ShaderEntry* entry;
  ObjectList* olist;
  GLuint program = 0;

  for(slist = pipeline->shaders;
      slist != NULL;
      slist = slist->next)
  {
    entry = slist->e;
    if(entry->n_objects < 1)
      continue;

    program =
    _ds_shader_get_pid(entry->shader);

  /*
   * Prepare mvp
   *
   */

    __gl_try_catch(
      glUseProgram(program);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    __gl_try_catch(
      ctx->mvpl = glGetUniformLocation(program, "a_mvp");
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

  /*
   * Compile
   *
   */

    /* use program call */
    _ds_jit_compile_call
    (ctx,
     G_CALLBACK(glUseProgram),
     TRUE,
     1,
     (guintptr) program);

    /* propagate compile */
    for(olist = entry->objects;
        olist != NULL;
        olist = olist->next)
    {
      success =
      ds_renderable_compile
      (olist->object,
       (DsRenderState*)
       ctx,
       program,
       cancellable,
       &tmp_err);

      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }
  }

  /* finalize code */
  _ds_jit_compile_end(ctx);

_error_:
  if G_LIKELY(success == TRUE)
  {
    pipeline->main = jitmain;
    pipeline->modified = FALSE;
  }
return success;
}

void
ds_pipeline_execute(DsPipeline* pipeline)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  JitState* ctx = &(pipeline->ctx);

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
  pipeline->main(pipeline, &(ctx->mvps), &tmp_err);
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

void
ds_pipeline_mvps_set_projection(DsPipeline *pipeline,
                                gfloat     *projection)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  JitState* ctx = &(pipeline->ctx);

  glm_mat4_copy((gpointer) projection, ctx->mvps.projection);
}

void
ds_pipeline_mvps_set_view(DsPipeline *pipeline,
                          gfloat     *view)
{
  g_return_if_fail(DS_IS_PIPELINE(pipeline));
  JitState* ctx = &(pipeline->ctx);

  glm_mat4_copy((gpointer) view, ctx->mvps.view);
}
