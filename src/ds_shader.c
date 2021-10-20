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

/**
 * SECTION:dsshader
 * @Short_description: a OpenGL program
 * @Title: DsShader
 *
 * DsShader encapsulates complexities of OpenGL shader source
 * loading, compilation and program linking, it's lifespan and even
 * caching.
 *
 */

G_DEFINE_QUARK(ds-shader-error-quark,
               ds_shader_error);

static
void ds_shader_g_initable_iface_init(GInitableIface* iface);

typedef struct _Shaders Shaders;

G_GNUC_INTERNAL
gboolean
_ds_shader_cache_try_load(DsCacheProvider  *cache_provider,
                          GLuint            program,
                          guint             source_hash,
                          GCancellable     *cancellable,
                          GError          **error);
G_GNUC_INTERNAL
gboolean
_ds_shader_cache_try_save(DsCacheProvider  *cache_provider,
                          GLuint            program,
                          guint             source_hash,
                          GCancellable     *cancellable,
                          GError          **error);

/*
 * Structs
 *
 */

struct _Shaders
{
  GLuint sid;
  GBytes* source;
};

static const GLenum
shader_types[] =
{
  GL_VERTEX_SHADER,
  GL_FRAGMENT_SHADER,
  GL_GEOMETRY_SHADER,
};

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
  DsCacheProvider* cache_provider;

  /*<private>*/
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

  /*<private>*/
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

G_STATIC_ASSERT(G_N_ELEMENTS(shader_types) == n_shaders);

enum {
  prop_0,
  prop_vertex_file,
  prop_fragment_file,
  prop_geometry_file,
  prop_vertex_stream,
  prop_fragment_stream,
  prop_geometry_stream,
  prop_cache_provider,
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
  ds_shader_g_initable_iface_init));

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
    if(file != NULL)
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

static gboolean
load_shader_sources(DsShader* self, Shaders* shaders, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GMemoryOutputStream* mem = NULL;
  guint i;

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
      g_propagate_error(error, tmp_err);
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
      g_propagate_error(error, tmp_err);
      goto_error();
    }

  /*
   * Steal bytes
   *
   */

    shaders[i].source =
    g_memory_output_stream_steal_as_bytes(mem);

  /*
   * Clean up
   *
   */
    g_clear_object(&(self->streams[i]));
    g_clear_object(&(self->files[i]));
  }

_error_:
  g_clear_object(&mem);
return success;
}

static guint
hash_sources(Shaders* shaders)
{
  guint hash = 0;
  guint i;

  for(i = 0;i < n_shaders;i++)
  if(shaders[i].source != NULL)
  {
    hash ^= g_bytes_hash(shaders[i].source);
  }
return hash;
}

static gboolean
compile_from_source(GLuint pid, Shaders* shaders, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GLint return_ = FALSE;
  GLint infolen = 0;
  GLuint sid;
  guint i;

/*
 * Compile shader and
 * attach them to program
 *
 */

  for(i = 0;i < n_shaders;i++)
  {
    if(shaders[i].source == NULL)
      continue;

  /*
   * Create shaders
   *
   */

    __gl_try_catch(
      sid = glCreateShader(shader_types[i]);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    if G_UNLIKELY(sid == 0)
    {
      g_set_error_literal
      (error,
       DS_SHADER_ERROR,
       DS_SHADER_ERROR_FAILED,
       "Invalid shader object\r\n");
      goto_error();
    }

    const GLchar* sources[] = {g_bytes_get_data(shaders[i].source, NULL), NULL};
    const GLint lengths[] = {g_bytes_get_size(shaders[i].source), 0};

  /*
   * Load source
   *
   */

    __gl_try_catch(
      glShaderSource(sid, 1, sources, lengths);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

  /*
   * Compile
   *
   */

    __gl_try_catch(
      glCompileShader(sid);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    __gl_try_catch(
      glGetShaderiv(sid, GL_COMPILE_STATUS, &return_);
      glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &infolen);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    if G_UNLIKELY(return_ == FALSE)
    {
      g_assert(infolen != 0);
      gchar* msg = g_malloc(infolen + 1);

      __gl_try_catch(
        glGetShaderInfoLog(sid, infolen, NULL, msg);
      ,
        g_propagate_error(error, glerror);
        _g_free0(msg);
        goto_error();
      );

      g_set_error_literal
      (error,
       DS_SHADER_ERROR,
       DS_SHADER_ERROR_INVALID_SHADER,
       msg);
      _g_free0(msg);
      goto_error();
    }

  /*
   * Attach shader
   *
   */

    __gl_try_catch(
      glAttachShader(pid, sid);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

  /*
   * Finalize shader
   *
   */

    shaders[i].sid = sid;
    sid = 0;
  }

/*
 * Link program
 *
 */

  __gl_try_catch(
    glLinkProgram(pid);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glGetProgramiv(pid, GL_LINK_STATUS, &return_);
    glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &infolen);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(return_ == FALSE)
  {
    g_assert(infolen != 0);
    gchar* msg = g_malloc(infolen + 1);

    __gl_try_catch(
      glGetProgramInfoLog(pid, infolen, NULL, msg);
    ,
      g_propagate_error(error, glerror);
      _g_free0(msg);
      goto_error();
    );

    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_INVALID_PROGRAM,
     msg);
    _g_free0(msg);
    goto_error();
  }

/*
 * Detach shaders
 *
 */

  for(i = 0;i < n_shaders;i++)
  {
    if(shaders[i].sid == 0)
      continue;

    __gl_try_catch(
      glDetachShader(pid, shaders[i].sid);
      shaders[i].sid = 0;
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

_error_:
  if G_UNLIKELY(sid != 0)
    glDeleteShader(sid);
return success;
}

static gboolean
ds_shader_g_initable_iface_init_sync(GInitable     *pself,
                                     GCancellable  *cancellable,
                                     GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsShader* self = DS_SHADER(pself);
  GLint return_ = FALSE;
  GLint infolen = 0;
  GLuint pid;
  guint i;

  Shaders shaders[n_shaders] = {0};
  guint source_hash;

/*
 * Get cache provider
 *
 */

  if G_UNLIKELY
    (self->cache_provider == NULL)
  {
    self->cache_provider =
    ds_cache_provider_get_default();

    if G_UNLIKELY
      (self->cache_provider == NULL)
    {
      g_set_error_literal
      (error,
       DS_FOLDER_PROVIDER_ERROR,
       DS_FOLDER_PROVIDER_ERROR_INVALID,
       "Invalid default cache provider\r\n");
      goto_error();
    }
    else
    {
      g_object_ref(self->cache_provider);
    }
  }

/*
 * Load sources
 *
 */

  success =
  load_shader_sources(self, shaders, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }
  else
  {
    source_hash = hash_sources(shaders);
    g_assert(source_hash != 0);
  }

/*
 * Create program
 *
 */

  __gl_try_catch(
    pid = glCreateProgram();
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(pid == 0)
  {
    g_set_error_literal
    (error,
     DS_SHADER_ERROR,
     DS_SHADER_ERROR_FAILED,
     "Invalid program object\r\n");
    goto_error();
  }

/*
 * Load program
 *
 */

#if !DEBUG
  /* try load from cache */
  success =
  _ds_shader_cache_try_load(self->cache_provider, pid, source_hash, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    if(g_error_matches(tmp_err, DS_SHADER_ERROR, DS_SHADER_ERROR_INVALID_BINARY))
      g_clear_error(&tmp_err);
    else
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

  /* extra check */
  if G_UNLIKELY(success == TRUE)
  {
    __gl_try_catch(
      glGetProgramiv(pid, GL_LINK_STATUS, &return_);
      success = (return_ == TRUE);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

  /* load from source and cache it */
  if G_UNLIKELY(success == FALSE)
  {
#endif // !DEBUG

    success =
    compile_from_source(pid, shaders, cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }

#if !DEBUG
    success =
    _ds_shader_cache_try_save(self->cache_provider, pid, source_hash, cancellable, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }
#endif // !DEBUG

/*
 * Validate program
 *
 */

  __gl_try_catch(
    glValidateProgram(pid);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glGetProgramiv(pid, GL_VALIDATE_STATUS, &return_);
    glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &infolen);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(return_ == FALSE)
  {
    if(infolen > 0)
    {
      gchar* msg = g_malloc(infolen + 1);

      __gl_try_catch(
        glGetProgramInfoLog(pid, infolen, NULL, msg);
      ,
        g_propagate_error(error, glerror);
        _g_free0(msg);
        goto_error();
      );

      g_set_error_literal
      (error,
       DS_SHADER_ERROR,
       DS_SHADER_ERROR_INVALID_PROGRAM,
       msg);
      _g_free0(msg);
      goto_error();
    }
    else
    {
      g_set_error_literal
      (error,
       DS_SHADER_ERROR,
       DS_SHADER_ERROR_INVALID_PROGRAM,
       "Unknown error validating programr\r\n");
      goto_error();
    }
  }

/*
 * Finish program
 *
 */

  self->pid = pid;
  pid = 0;

_error_:
  if G_UNLIKELY(pid != 0)
    glDeleteProgram(pid);
  for(i = 0;i < n_shaders;i++)
  {
    glDeleteShader(shaders[i].sid);
    g_bytes_unref(shaders[i].source);
  }
return success;
}

static
void ds_shader_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_shader_g_initable_iface_init_sync;
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
  case prop_cache_provider:
    g_set_object(&(self->cache_provider), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_shader_class_finalize(GObject* pself) {
  DsShader* self = DS_SHADER(pself);

  __gl_try(
    glDeleteProgram(self->pid);
  );
  __gl_catch(
    g_critical
    ("%s: %i: %s\r\n",
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  ,);

G_OBJECT_CLASS(ds_shader_parent_class)->finalize(pself);
}

static
void ds_shader_class_dispose(GObject* pself) {
  DsShader* self = DS_SHADER(pself);

  g_clear_object(&(self->vertex_file));
  g_clear_object(&(self->fragment_file));
  g_clear_object(&(self->geometry_file));
  g_clear_object(&(self->vertex_stream));
  g_clear_object(&(self->fragment_stream));
  g_clear_object(&(self->geometry_stream));
  g_clear_object(&(self->cache_provider));
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

  properties[prop_cache_provider] =
    g_param_spec_object
    ("cache-provider",
     "cache-provider",
     "cache-provider",
     DS_TYPE_CACHE_PROVIDER,
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

/**
 * ds_shader_new: (constructor)
 * @vertex_file: (nullable): file from which load vertex shader source code.
 * @vertex_stream: (nullable): stream from which load vertex shader source code.
 * @fragment_file: (nullable): file from which load fragment shader source code.
 * @fragment_stream: (nullable): stream from which load fragment shader source code.
 * @geometry_file: (nullable): file from which load geometry shader source code.
 * @geometry_stream: (nullable): file from which load geometry shader source code.
 * @cache_provider: (nullable): cache provider object.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Creates a new #DsShader object, preforming this steps:
 * - Looks up for a cached program (looks up in directory provided by
 * @cache_provider), checks if is compatible and then checks if needs to
 * be updated. If all goes well, #DsShader is ready to use.
 * - If not, creates an empty OpenGL programs.
 * - Loads available shader code (those shader type which have a source).
 * - Compiles and link the program, then caches it.
 *
 * Returns: (transfer full): a new #DsShader instance.
 */
DsShader*
ds_shader_new(GFile            *vertex_file,
              GInputStream     *vertex_stream,
              GFile            *fragment_file,
              GInputStream     *fragment_stream,
              GFile            *geometry_file,
              GInputStream     *geometry_stream,
              DsCacheProvider  *cache_provider,
              GCancellable     *cancellable,
              GError          **error)
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
   "cache-provider", cache_provider,
   NULL);
}

/**
 * ds_shader_new_from_files: (constructor)
 * @vertex_file: (nullable): file from which load vertex shader source
 * code.
 * @fragment_file: (nullable): file from which load fragment shader source
 * code.
 * @geometry_file: (nullable): file from which load geometry shader source
 * code.
 * @cache_provider: (nullable): cache provider object.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Performs the same as #ds_shader_new(), but removes overhead of
 * pass NULL to stream inputs.
 *
 * Returns: (transfer full): a new #DsShader instance.
 */
DsShader*
ds_shader_new_from_files(GFile           *vertex_file,
                         GFile           *fragment_file,
                         GFile           *geometry_file,
                         DsCacheProvider *cache_provider,
                         GCancellable    *cancellable,
                         GError         **error)
{
  return (DsShader*)
  g_initable_new
  (DS_TYPE_SHADER,
   cancellable,
   error,
   "vertex-file", vertex_file,
   "fragment-file", fragment_file,
   "geometry-file", geometry_file,
   "cache-provider", cache_provider,
   NULL);
}

G_GNUC_INTERNAL
GLuint
_ds_shader_get_pid(DsShader *shader)
{
  g_return_val_if_fail(DS_IS_SHADER(shader), 0);
return shader->pid;
}
