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
#include <ds_dds.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_renderable.h>
#include <ds_skybox.h>
#include <SDL.h>
#include <SDL_image.h>

G_DEFINE_QUARK(ds-skybox-error-quark,
               ds_skybox_error);

static
void ds_skybox_class_base_init(DsSkyboxClass* klass);
static
void ds_skybox_class_base_fini(DsSkyboxClass* klass);
static
void ds_skybox_class_init(DsSkyboxClass* klass);
static
void ds_skybox_init(DsSkybox* self);
static
void ds_skybox_g_initable_iface_init(GInitableIface* iface);
static
void ds_skybox_ds_callable_iface_init(DsCallableIface* iface);
static
void ds_skybox_ds_renderable_iface_init(DsRenderableIface* iface);
static
gpointer ds_skybox_parent_class = NULL;

/*
 * Object definition
 *
 */

struct _DsSkybox
{
  GObject parent_instance;

  /*<private>*/
  GFile* source;
  gchar* name;
  GLuint vao;
  GLuint tio;

  union
  {
    GLuint bo_s[2];
    struct
    {
      GLuint vbo;
      GLuint ebo;
    };
  };
};

struct _DsSkyboxClass
{
  GObjectClass parent_class;

  /*<private>*/
  GRegex* pattern;
};

enum {
  prop_0,
  prop_source,
  prop_name,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

GType
ds_skybox_get_type()
{
  static
  GType skybox_type = 0;

  if G_UNLIKELY(skybox_type == 0)
  {
    const
    GTypeInfo type_info =
    {
      sizeof(DsSkyboxClass),
      (GBaseInitFunc)
      ds_skybox_class_base_init,
      (GBaseFinalizeFunc)
      ds_skybox_class_base_fini,
      (GClassInitFunc)
      ds_skybox_class_init,
      NULL,
      NULL,
      sizeof(DsSkybox),
      0,
      (GInstanceInitFunc)
      ds_skybox_init,
      NULL,
    };

    skybox_type =
    g_type_register_static
    (G_TYPE_OBJECT,
     g_intern_static_string
     ("DsSkybox"),
     &type_info,
     0);

#define g_define_type_id skybox_type

    G_IMPLEMENT_INTERFACE
    (G_TYPE_INITABLE,
     ds_skybox_g_initable_iface_init)

    G_IMPLEMENT_INTERFACE
    (DS_TYPE_CALLABLE,
     ds_skybox_ds_callable_iface_init)

    G_IMPLEMENT_INTERFACE
    (DS_TYPE_RENDERABLE,
     ds_skybox_ds_renderable_iface_init)

#undef g_define_type_id
  }
return skybox_type;
}

static
void ds_skybox_class_base_init(DsSkyboxClass* klass) {
  GError* tmp_err = NULL;

  klass->pattern =
  g_regex_new
  ("%[a-zA-Z]",
   0,
   0,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    g_assert_not_reached();
  }
}

static
void ds_skybox_class_base_fini(DsSkyboxClass* klass) {
  g_clear_pointer(&(klass->pattern), g_regex_unref);
}

static const
vec3 cube_vertices[] =
{
  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,

  -1.0f,  1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
};

static const
GLuint cube_indices[] =
{
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,
};

static const
GLuint cubmap_faces[] =
{
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

static
struct
{
  const gchar* str;
  const guint len;
} sides[] =
{
#define SIDE(str) str, sizeof( str ) - 1,
  SIDE("right")
  SIDE("left")
  SIDE("top")
  SIDE("bottom")
  SIDE("front")
  SIDE("back")
#undef SIDE
};

G_STATIC_ASSERT
(   G_N_ELEMENTS(cubmap_faces)
 == G_N_ELEMENTS(sides));

static gboolean
on_name_replace(const GMatchInfo *info,
                GString          *result,
                guint            *pi)
{
  const gchar* name =
  g_match_info_get_string(info);
  gint start, end_;
  guint i = *pi;

  g_match_info_fetch_pos(info, 0, &start, &end_);
  g_assert(start >= 0 && name[start] == '%');

  switch(name[start + 1])
  {
  case '%':
    g_string_append_c(result, '%');
  case 'i':
    g_string_printf(result, "i", i);
    break;
  case 's':
    g_string_append_len
    (result,
     sides[i].str,
     sides[i].len);
    break;
  default:
    g_warning
    ("Unknown scape sequence '%.*s'\r\n",
     end_ - start, name + start);
    break;
  }
}

static gboolean
load_skybox_faces(DsSkybox       *self,
                  GCancellable   *cancellable,
                  GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  guint i;

  for(i = 0;
      i < G_N_ELEMENTS(sides);
      i++)
  {
    gchar* filename =
    g_regex_replace_eval
    (DS_SKYBOX_GET_CLASS(self)->pattern,
     self->name,
     -1,
     0,
     0,
     (GRegexEvalCallback)
     on_name_replace,
     &i,
     &tmp_err);

    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      _g_free0(filename);
      goto_error();
    }

    GFile* file =
    g_file_get_child
    (self->source,
     filename);
    _g_free0(filename);

    success =
    ds_dds_load_image(file, cubmap_faces[i], cancellable, &tmp_err);
    g_object_unref(file);

    if G_UNLIKELY(tmp_err != NULL)
    {
      g_propagate_error(error, tmp_err);
      goto_error();
    }
  }

_error_:
return success;
}

static gboolean
ds_skybox_g_initable_iface_init_sync(GInitable      *pself,
                                     GCancellable   *cancellable,
                                     GError        **error)
{
  DsSkybox* self = DS_SKYBOX(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Generate GL objects
 *
 */

  __gl_try_catch(
    glGenVertexArrays(1, &(self->vao));
    glGenTextures(1, &(self->tio));
    glGenBuffers(G_N_ELEMENTS(self->bo_s), self->bo_s);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

/*
 * Select objects
 *
 */

  __gl_try_catch(
    glBindVertexArray(self->vao);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

/*
 * Copy vertex array data
 *
 */

  /* vertices */
  __gl_try_catch(
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

  /* indices */
  __gl_try_catch(
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

/*
 * Enable vertex attributes
 *
 */

  __gl_try_catch(
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

/*
 * Unbind things
 *
 */

  __gl_try_catch(
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

/*
 * Load textures
 *
 */

  __gl_try_catch(
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->tio);
  ,
    g_propagate_error(error, tmp_err);
    goto_error();
  );

  success =
  load_skybox_faces(self, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  __gl_try_catch(
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

_error_:
  g_clear_object(&(self->source));
  g_clear_pointer(&(self->name), g_free);

  __gl_try_catch(
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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

static
void ds_skybox_g_initable_iface_init(GInitableIface* iface) {
  iface->init = ds_skybox_g_initable_iface_init_sync;
}

static
void ds_skybox_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(ds_skybox_new),
   ds_cclosure_marshal_OBJECT__OBJECT_STRING_OBJECT_POINTER,
   ds_cclosure_marshal_OBJECT__OBJECT_STRING_OBJECT_POINTERv,
   G_TYPE_OBJECT,
   4,
   G_TYPE_FILE,
   G_TYPE_STRING,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static gboolean
ds_skybox_ds_renderable_iface_compile(DsRenderable   *pself,
                                      DsRenderState  *state,
                                      GLuint          program,
                                      GCancellable   *cancellable,
                                      GError        **error)
{
  DsSkybox* self = DS_SKYBOX(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GLint uloc = 0;

/*
 * Fill uniform
 *
 */

  /* get uniform */
  __gl_try_catch(
    uloc = glGetUniformLocation(program, "a_skybox");
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  if G_UNLIKELY(uloc == (-1))
  {
    g_set_error_literal
    (error,
     DS_GL_ERROR,
     DS_GL_ERROR_FAILED,
     "Invalid uniform location\r\n");
    goto_error();
  }

  /* select texture unit */
  __gl_try_catch(
    glUniform1i(uloc, 0);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Compile calls
 *
 */

  /* update depth function */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glDepthFunc),
   1,
   (guintptr) GL_LEQUAL);

  /* activate texture */
#if GL_VERSION_4_5 == 1
  /* bind texture to texture unit */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindTextureUnit),
   2,
   (guintptr) 0,
   (guintptr) self->tio);
#else // GL_VERSION_4_5
# if GL_ARB_direct_state_access == 1
  if G_LIKELY(GLEW_ARB_direct_state_access == TRUE)
  {
    /* bind texture to texture unit */
    ds_render_state_pcall
    (state,
     G_CALLBACK(glBindTextureUnit),
     2,
     (guintptr) 0,
     (guintptr) self->tio);
    g_print("texture unit\r\n");
  }
  else
  {
# endif // GL_ARB_direct_state_access
    /* activate texture unit */
    ds_render_state_pcall
    (state,
     G_CALLBACK(glActiveTexture),
     1,
     (guintptr) GL_TEXTURE0);

    /* bind texture to texture unit */
    ds_render_state_pcall
    (state,
     G_CALLBACK(glBindTexture),
     2,
     (guintptr) GL_TEXTURE_CUBE_MAP,
     (guintptr) self->tio);
# if GL_ARB_direct_state_access == 1
  }
# endif // GL_ARB_direct_state_access
#endif // GL_VERSION_4_5

  /* bind vertex array */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexArray),
   1,
   (guintptr) self->vao);

  /* draw */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glDrawArrays),
   3,
   (guintptr) GL_TRIANGLES,
   (guintptr) 0,
   (guintptr) G_N_ELEMENTS(cube_vertices));

  /* unbind vertex array */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexArray),
   1,
   (guintptr) 0);

  /* unbind texture */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindTexture),
   2,
   (guintptr) GL_TEXTURE_CUBE_MAP,
   (guintptr) 0);

  /* restore depth function */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glDepthFunc),
   1,
   (guintptr) GL_LESS);

_error_:
  __gl_try(
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  );
  __gl_catch(
    g_warning
    ("(%s:%i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  ,);
return success;
}

static
void ds_skybox_ds_renderable_iface_init(DsRenderableIface* iface) {
  iface->compile = ds_skybox_ds_renderable_iface_compile;
}

static
void ds_skybox_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  DsSkybox* self = DS_SKYBOX(pself);
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
void ds_skybox_class_finalize(GObject* pself) {
  DsSkybox* self = DS_SKYBOX(pself);
  _g_free0(self->name);

  __gl_try(
    glDeleteVertexArrays(1, &(self->vao));
    glDeleteTextures(1, &(self->tio));
    glDeleteBuffers(G_N_ELEMENTS(self->bo_s), self->bo_s);
  );
  __gl_catch(
    g_warning
    ("(%s:%i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  ,);

G_OBJECT_CLASS(ds_skybox_parent_class)->finalize(pself);
}

static
void ds_skybox_class_dispose(GObject* pself) {
  DsSkybox* self = DS_SKYBOX(pself);
  g_clear_object(&(self->source));
G_OBJECT_CLASS(ds_skybox_parent_class)->dispose(pself);
}

static
void ds_skybox_class_init(DsSkyboxClass* klass) {
  ds_skybox_parent_class = g_type_class_peek_parent(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */

  oclass->set_property = ds_skybox_class_set_property;
  oclass->finalize = ds_skybox_class_finalize;
  oclass->dispose = ds_skybox_class_dispose;

/*
 * Porperties
 *
 */

  properties[prop_source] =
    g_param_spec_object
    (_TRIPLET("source"),
     G_TYPE_FILE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_name] =
    g_param_spec_string
    (_TRIPLET("name"),
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
void ds_skybox_init(DsSkybox* self) {
}

/*
 * Object methods
 *
 */

DsSkybox*
ds_skybox_new(GFile        *source,
              const gchar  *name,
              GCancellable *cancellable,
              GError      **error)
{
  return (DsSkybox*)
  g_initable_new
  (DS_TYPE_SKYBOX,
   cancellable,
   error,
   "source", source,
   "name", name,
   NULL);
}
