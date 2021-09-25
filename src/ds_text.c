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
#include <ds_font.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_renderable.h>
#include <ds_text.h>

G_DEFINE_QUARK(ds-text-error-quark,
               ds_text_error);

static
void ds_text_ds_callable_iface_init(DsCallableIface* iface);
static
void ds_text_ds_renderable_iface_init(DsRenderableIface* iface);

typedef union  _DsTextList  DsTextList;
typedef struct _DsTextEntry DsTextEntry;

/*
 * Object definition
 *
 */

struct _DsText
{
  GObject parent_instance;

  /*<private>*/
  GLuint tio;
  DsFont* font;
  union _DsTextList
  {
    GList list_;
    struct
    {
      struct _DsTextEntry
      {
        GLuint vao;
        GLuint vbo;
        gsize nvt;
      } *c;

      DsTextList* next;
      DsTextList* prev;
    };
  } *texts;

  mat4 scale;
};

enum {
  prop_0,
  prop_provider,
  prop_number,
};

G_STATIC_ASSERT(sizeof(DsTextHandle) == sizeof(gpointer));

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsText,
 ds_text,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_CALLABLE,
  ds_text_ds_callable_iface_init)
 G_IMPLEMENT_INTERFACE
 (DS_TYPE_RENDERABLE,
  ds_text_ds_renderable_iface_init));

static DsText*
_callable_new(gpointer  null_,
              DsFont   *provider)
{
  return
  ds_text_new(provider);
}

static DsTextHandle
_ds_text_print(DsText* text, const gchar* text_, gfloat x, gfloat y, GCancellable* cancellable, GError** error)
{
  vec2 pos = {x, y};
  return
  ds_text_print(text, text_, pos, cancellable, error);
}

static
void ds_text_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(_callable_new),
   ds_cclosure_marshal_OBJECT__OBJECT,
   ds_cclosure_marshal_OBJECT__OBJECTv,
   DS_TYPE_TEXT,
   1,
   DS_TYPE_FONT,
   G_TYPE_NONE);
  ds_callable_iface_add_method
  (iface,
   "print",
   DS_CLOSURE_FLAGS_NONE,
   G_CALLBACK(_ds_text_print),
   g_cclosure_marshal_generic,
   g_cclosure_marshal_generic_va,
   G_TYPE_POINTER,
   5,
   G_TYPE_STRING,
   G_TYPE_FLOAT,
   G_TYPE_FLOAT,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static gboolean
ds_text_ds_renderable_iface_compile(DsRenderable* pself, DsRenderState* state, GLuint program, GCancellable* cancellable, GError** error)
{
  DsText* self = DS_TEXT(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsTextList* list;
  DsTextEntry* entry;
  GLint uloc = 0;

/*
 * Fill uniforms
 *
 */

  /* get uniform */
  __gl_try_catch(
    uloc = glGetUniformLocation(program, "a_charmap");
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

  /* get uniform */
  __gl_try_catch(
    uloc = glGetUniformLocation(program, "a_color");
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

  vec3 color = {1.f, 1.f, 1.f};

  /* set text color */
  __gl_try_catch(
    glUniform3fv(uloc, 1, color);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

/*
 * Compile calls
 *
 */

  ds_render_state_mvp_set_model
  (state,
   (gfloat*)
   self->scale);

  /* activate texture */
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
   (guintptr) GL_TEXTURE_2D,
   (guintptr) self->tio);

  for(list = self->texts;
      list != NULL;
      list = list->next)
  {
    /* bind vertex array */
    ds_render_state_pcall
    (state,
     G_CALLBACK(glBindVertexArray),
     1,
     (guintptr) list->c->vao);

    /* draw */
    ds_render_state_pcall
    (state,
     G_CALLBACK(glDrawArrays),
     3,
     (guintptr) GL_TRIANGLES,
     (guintptr) 0,
     (guintptr) list->c->nvt);
  }

  /* unbind vertex array */
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexArray),
   1,
   (guintptr) 0);

_error_:
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

static
void ds_text_ds_renderable_iface_init(DsRenderableIface* iface) {
  iface->compile = ds_text_ds_renderable_iface_compile;
}

static
void ds_text_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec) {
  DsText* self = DS_TEXT(pself);
  switch(prop_id)
  {
  case prop_provider:
    g_set_object(&(self->font), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_text_class_constructed(GObject* pself) {
  DsText* self = DS_TEXT(pself);
  g_object_get(self->font, "texture-id", &(self->tio), NULL);
G_OBJECT_CLASS(ds_text_parent_class)->constructed(pself);
}

static void
run_finalize(DsText* self, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  DsTextList* list;
  for(list = self->texts;
      list != NULL;
      list = list->next)
  {
    __gl_try_catch(
      glDeleteVertexArrays(1, &(list->c->vao));
      glDeleteBuffers(1, &(list->c->vbo));
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    g_slice_free(DsTextEntry, list->c);
    list->c = NULL;
  }

_error_:
return;
}

static
void ds_text_class_finalize(GObject* pself) {
  GError* tmp_err = NULL;
  run_finalize(DS_TEXT(pself), &tmp_err);
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
G_OBJECT_CLASS(ds_text_parent_class)->finalize(pself);
}

static
void ds_text_class_dispose(GObject* pself) {
  DsText* self = DS_TEXT(pself);
  g_clear_object(&(self->font));
G_OBJECT_CLASS(ds_text_parent_class)->dispose(pself);
}

static
void ds_text_class_init(DsTextClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */

  oclass->set_property = ds_text_class_set_property;
  oclass->constructed = ds_text_class_constructed;
  oclass->finalize = ds_text_class_finalize;
  oclass->dispose = ds_text_class_dispose;

/*
 * properties
 *
 */

  properties[prop_provider] =
    g_param_spec_object
    (_TRIPLET("provider"),
     DS_TYPE_FONT,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static
void ds_text_init(DsText* self) {
  glm_mat4_identity(self->scale);
}

/*
 * Object methods
 *
 */

DsText*
ds_text_new(DsFont* provider)
{
  return (DsText*)
  g_object_new(DS_TYPE_TEXT, "provider", provider, NULL);
}

G_GNUC_INTERNAL
gboolean
_ds_font_generate_vao(DsFont         *font,
                      const gchar    *text,
                      vec2            position,
                      GLuint         *pvao,
                      GLuint         *pvbo,
                      gsize          *pnvt,
                      GCancellable   *cancellable,
                      GError        **error);

DsTextHandle
ds_text_print(DsText         *text,
              const gchar    *text_,
              vec2            position,
              GCancellable   *cancellable,
              GError        **error)
{
  g_return_val_if_fail(DS_TEXT(text), NULL);
  g_return_val_if_fail(text_ != NULL, NULL);
  g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  DsTextHandle handle = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  GLuint vao = 0, vbo = 0;
  gsize n_vertices = 0;

  success =
  g_utf8_validate(text_, -1, NULL);
  if G_UNLIKELY(success == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_UNKNOWN_GLYPH,
     "Invalid UTF-8 string\r\n");
    goto_error();
  }

  success =
  _ds_font_generate_vao(text->font, text_, position, &vao, &vbo, &n_vertices, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  DsTextEntry* data =
  g_slice_new(DsTextEntry);

  data->vao = ds_steal_handle_id(&vao);
  data->vbo = ds_steal_handle_id(&vbo);
  data->nvt = n_vertices;

  text->texts =
  (DsTextList*)
  g_list_concat
  (&(text->texts->list_),
   g_list_append
   (NULL,
    (gpointer)
    data));

  handle = (DsTextHandle) data;
_error_:
  if G_UNLIKELY(success == FALSE)
    handle = NULL;
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
return handle;
}
