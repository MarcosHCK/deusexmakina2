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
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_shader.h>

G_DEFINE_QUARK(ds-shader-error-quark,
               ds_shader_error);

static
void ds_shader_g_initable_iface_init(GInitableIface* iface);
static
void ds_shader_g_async_initable_iface_init(GAsyncInitableIface* iface);

/*
 * Object definition
 *
 */

#define n_shaders 3

struct _DsShader
{
  GObject parent_instance;

  /*<private>*/
  GLuint pid;

  union
  {
    GFile* files[n_shaders];
    struct
    {
      GFile* vertex_file;
      GFile* fragment_file;
      GFile* geometry_file;
    };
  };

  union
  {
    GInputStream* streams[n_shaders];
    struct
    {
      GInputStream* vertex_stream;
      GInputStream* fragment_stream;
      GInputStream* geometry_stream;
    };
  };
};

const GLenum
shader_types[] = {
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
  GL_GEOMETRY_SHADER,
};

G_STATIC_ASSERT(G_N_ELEMENTS(shader_types) == n_shaders);

enum {
  prop_0,
  prop_vertex_file,
  prop_fragment_file,
  prop_geometry_file,
  prop_vertex_stream,
  prop_fragment_stream,
  prop_geometry_stream,
  prop_number,
};

G_STATIC_ASSERT(n_shaders == (prop_number - 1) / 2);

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsShader,
 ds_shader,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_shader_g_initable_iface_init)
 G_IMPLEMENT_INTERFACE
 (G_TYPE_ASYNC_INITABLE,
  ds_shader_g_async_initable_iface_init));

static inline gboolean
assert_stream(GFile          *file,
              GInputStream  **pstream,
              GCancellable   *cancellable,
              GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  if(*pstream == NULL)
  {
    if(file == NULL)
    {
      (*pstream) = (GInputStream*)
      g_file_read(file, cancellable, &tmp_err);
      if G_UNLIKELY(tmp_err != NULL)
      {
        g_propagate_error(error, tmp_err);
        goto_error();
      }
    }
  }

_error_:
return success;
}

static
void init_fn(GTask         *task,
             DsShader      *self,
             gpointer       task_data,
             GCancellable  *cancellable)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GMemoryOutputStream* mem = NULL;
  GBytes* bytes = NULL;
  GLuint pid, sid;
  GLint return_ = FALSE;
  GLint infolen = 0;
  guint i;

  pid = glCreateProgram();
  if G_UNLIKELY(pid == 0)
  {
    g_task_return_error
    (task, ds_gl_get_error());
    goto_error();
  }

  GLuint* sids = g_newa(GLuint, n_shaders);
  memset(sids, 0, sizeof(GLuint) * n_shaders);

  for(i = 0;i < n_shaders;i++)
  {

  /*
   * Get stream
   *
   */

    assert_stream
    (self->files[i],
     &(self->streams[i]),
     cancellable,
     &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_task_return_error(task, tmp_err);
      goto_error();
    }

    if(self->streams[i] == NULL)
      continue;

  /*
   * Create target stream
   *
   */

    g_clear_object(&mem);
    mem = (GMemoryOutputStream*)
    g_memory_output_stream_new_resizable();

  /*
   * Copy shader source
   *
   */

    g_output_stream_splice
    (G_OUTPUT_STREAM(mem),
     self->streams[i],
     G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
     cancellable,
     &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_task_return_error(task, tmp_err);
      goto_error();
    }

  /*
   * Steal bytes
   *
   */

    g_clear_pointer
    (&bytes, g_bytes_unref);
    bytes =
    g_memory_output_stream_steal_as_bytes(mem);

    const GLchar* sources[] = {g_bytes_get_data(bytes, NULL), NULL};
    const GLint lengths[] = {g_bytes_get_size(bytes), 0};

    g_clear_object(&(self->streams[i]));
    g_clear_object(&(self->files[i]));

  /*
   * Compile shader
   *
   */

    sid =
    glCreateShader(shader_types[i]);
    if G_UNLIKELY(sid == 0)
    {
      g_task_return_error
      (task, ds_gl_get_error());
      goto_error();
    }

    glShaderSource(sid, 1, sources, lengths);
    glCompileShader(sid);

  /*
   * Check compilation status
   *
   */

    glGetShaderiv(sid, GL_COMPILE_STATUS, &return_);
    glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &infolen);

    if G_UNLIKELY(return_ == GL_FALSE)
    {
      g_assert(infolen > 0);
      gchar* msg = g_malloc(infolen + 1);
      glGetShaderInfoLog(sid, infolen, NULL, msg);

      g_task_return_error
      (task,
       g_error_new_literal
       (DS_SHADER_ERROR,
        DS_SHADER_ERROR_COMPILE,
        msg));
      goto_error();
    }

  /*
   * Attach shader to program
   *
   */

    glAttachShader(pid, sid);
    sids[i] = sid;
  }

  /*
   * Link program
   *
   */

  glLinkProgram(pid);

  /*
   * Check link status
   *
   */

  glGetShaderiv(sid, GL_LINK_STATUS, &return_);
  glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &infolen);

  if G_UNLIKELY(return_ == GL_FALSE)
  {
    g_assert(infolen > 0);
    gchar* msg = g_malloc(infolen + 1);
    glGetShaderInfoLog(sid, infolen, NULL, msg);

    g_task_return_error
    (task,
     g_error_new_literal
     (DS_SHADER_ERROR,
      DS_SHADER_ERROR_COMPILE,
      msg));
    goto_error();
  }

  /*
   * Detach shaders
   *
   */
  for(i = 0;i < n_shaders;i++)
  {
    glDetachShader(pid, sids[i]);
  }

  /*
   * Finish program
   *
   */
  self->pid = pid; pid = 0;

_error_:
  if G_UNLIKELY(success == TRUE)
    g_task_return_boolean(task, success);
  if G_UNLIKELY(pid != 0)
    glDeleteProgram(pid);
  for(i = 0;i < n_shaders;i++)
    glDeleteShader(sids[i]);
  g_clear_pointer
  (&bytes, g_bytes_unref);
  g_clear_object(&mem);
}

static gboolean
ds_shader_g_initable_iface_init_sync(GInitable     *initable,
                                     GCancellable  *cancellable,
                                     GError       **error)
{
  GTask* task =
  g_task_new
  (G_OBJECT(initable),
   cancellable,
   NULL, NULL);

  g_task_set_name(task, "DsShader::init");
  g_task_set_priority(task, G_PRIORITY_DEFAULT);
  g_task_run_in_thread_sync(task, (GTaskThreadFunc) init_fn);
  gboolean return_ = g_task_propagate_boolean(task, error);
  g_object_unref(task);
return return_;
}

static
void ds_shader_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_shader_g_initable_iface_init_sync;
}

static void
ds_shader_g_initable_iface_init_async(GAsyncInitable       *initable,
                                      int                   io_priority,
                                      GCancellable         *cancellable,
                                      GAsyncReadyCallback   callback,
                                      gpointer              user_data)
{
  GTask* task =
  g_task_new
  (G_OBJECT(initable),
   cancellable,
   callback,
   user_data);

  g_task_set_name(task, "DsShader::init_async");
  g_task_set_priority(task, io_priority);
  g_task_run_in_thread(task, (GTaskThreadFunc) init_fn);
  g_object_unref(task);
}

static gboolean
ds_shader_g_initable_iface_init_finish(GAsyncInitable  *initable,
                                       GAsyncResult    *res,
                                       GError         **error)
{
  return g_task_propagate_boolean(G_TASK(res), error);
}

static
void ds_shader_g_async_initable_iface_init(GAsyncInitableIface* iface) {
  iface->init_async = ds_shader_g_initable_iface_init_async;
  iface->init_finish = ds_shader_g_initable_iface_init_finish;
}

static
void ds_shader_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  DsShader* self = DS_SHADER(pself);
  switch(prop_id)
  {
  case prop_vertex_file:
    g_set_object(&(self->vertex_file), g_value_get_object(value));
    break;
  case prop_fragment_file:
    g_set_object(&(self->fragment_file), g_value_get_object(value));
    break;
  case prop_geometry_file:
    g_set_object(&(self->geometry_file), g_value_get_object(value));
    break;
  case prop_vertex_stream:
    g_set_object(&(self->vertex_stream), g_value_get_object(value));
    break;
  case prop_fragment_stream:
    g_set_object(&(self->fragment_stream), g_value_get_object(value));
    break;
  case prop_geometry_stream:
    g_set_object(&(self->geometry_stream), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_shader_class_finalize(GObject* pself) {
  DsShader* self = DS_SHADER(pself);

/*
 * Finalize
 *
 */
  glDeleteProgram(self->pid);

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(ds_shader_parent_class)->finalize(pself);
}

static
void ds_shader_class_dispose(GObject* pself) {
  DsShader* self = DS_SHADER(pself);

/*
 * Dispose
 *
 */
  g_clear_object(&(self->vertex_file));
  g_clear_object(&(self->fragment_file));
  g_clear_object(&(self->geometry_file));
  g_clear_object(&(self->vertex_stream));
  g_clear_object(&(self->fragment_stream));
  g_clear_object(&(self->geometry_stream));

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(ds_shader_parent_class)->dispose(pself);
}

static
void ds_shader_class_init(DsShaderClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  oclass->set_property = ds_shader_class_set_property;
  oclass->finalize = ds_shader_class_finalize;
  oclass->dispose = ds_shader_class_dispose;

/*
 * Properties
 *
 */
  properties[prop_vertex_file] =
    g_param_spec_object
    ("vertex-file",
     "vertex-file",
     "vertex-file",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  properties[prop_fragment_file] =
    g_param_spec_object
    ("fragment-file",
     "fragment-file",
     "fragment-file",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  properties[prop_geometry_file] =
    g_param_spec_object
    ("geometry-file",
     "geometry-file",
     "geometry-file",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  properties[prop_vertex_stream] =
    g_param_spec_object
    ("vertex-stream",
     "vertex-stream",
     "vertex-stream",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  properties[prop_fragment_stream] =
    g_param_spec_object
    ("fragment-stream",
     "fragment-stream",
     "fragment-stream",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  properties[prop_geometry_stream] =
    g_param_spec_object
    ("geometry-stream",
     "geometry-stream",
     "geometry-stream",
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static
void ds_shader_init(DsShader* self) {
}

/*
 * Object methods
 *
 */

DsShader*
ds_shader_new(GFile        *vertex_file,
              GInputStream *vertex_stream,
              GFile        *fragment_file,
              GInputStream *fragment_stream,
              GFile        *geometry_file,
              GInputStream *geometry_stream,
              GCancellable *cancellable,
              GError      **error)
{
  return (DsShader*)
  g_initable_new
  (DS_TYPE_SHADER,
   cancellable,
   error,
   "vertex-file", vertex_file,
   "vertex-stream", vertex_stream,
   "fragment-file", fragment_file,
   "fragment-stream", fragment_stream,
   "geometry-file", geometry_file,
   "geometry-stream", geometry_stream,
   NULL);
}

void
ds_shader_new_async(GFile                *vertex_file,
                    GInputStream         *vertex_stream,
                    GFile                *fragment_file,
                    GInputStream         *fragment_stream,
                    GFile                *geometry_file,
                    GInputStream         *geometry_stream,
                    int                   io_priority,
                    GCancellable         *cancellable,
                    GAsyncReadyCallback   callback,
                    gpointer              user_data)
{
  g_async_initable_new_async
  (DS_TYPE_SHADER,
   io_priority,
   cancellable,
   callback,
   user_data,
   "vertex-file", vertex_file,
   "vertex-stream", vertex_stream,
   "fragment-file", fragment_file,
   "fragment-stream", fragment_stream,
   "geometry-file", geometry_file,
   "geometry-stream", geometry_stream,
   NULL);
}

DsShader*
ds_shader_new_finish(GAsyncResult  *res,
                     GError       **error)
{
  return (DsShader*)
  g_async_initable_new_finish
  (g_task_get_source_object(G_TASK(res)),
   res,
   error);
}

gboolean
ds_shader_use(DsShader     *shader,
              GError      **error)
{
  g_return_val_if_fail(DS_IS_SHADER(shader), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
  GLuint pid = shader->pid;
  gboolean success = TRUE;

  g_return_val_if_fail(pid != 0, FALSE);
  glUseProgram(pid);

  if G_UNLIKELY
    (glGetError() != GL_NO_ERROR)
  {
    g_propagate_error(error, ds_gl_get_error());
    goto_error();
  }
_error_:
return success;
}
