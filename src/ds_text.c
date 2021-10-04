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

static mat4 scale;

static void
ds_text_entry_free(DsTextEntry* entry);

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
        gint df;
      } *c;

      DsTextList* next;
      DsTextList* prev;
    };
  } *texts;
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
  ds_text_ds_renderable_iface_init)
 {
   glm_mat4_identity(scale);
 });

static DsTextHandle
_ds_text_print(DsText* text, DsTextHandle handle, const gchar* text_, gfloat x, gfloat y, GCancellable* cancellable, GError** error)
{
  vec2 pos = {x, y};
  return
  ds_text_print(text, handle, text_, pos, cancellable, error);
}

static
void ds_text_ds_callable_iface_init(DsCallableIface* iface) {
  ds_callable_iface_add_method
  (iface,
   "new",
   DS_CLOSURE_CONSTRUCTOR,
   G_CALLBACK(ds_text_new),
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
   ds_cclosure_marshal_POINTER__INSTANCE_POINTER_STRING_FLOAT_FLOAT_OBJECT_POINTER,
   ds_cclosure_marshal_POINTER__INSTANCE_POINTER_STRING_FLOAT_FLOAT_OBJECT_POINTERv,
   G_TYPE_POINTER,
   6,
   G_TYPE_POINTER,
   G_TYPE_STRING,
   G_TYPE_FLOAT,
   G_TYPE_FLOAT,
   G_TYPE_CANCELLABLE,
   G_TYPE_POINTER);
}

static void
glBindVertexArray_s(GLuint* pvao)
{
  glBindVertexArray(*pvao);
}

static void
glDrawArrays_s(GLenum mode, GLint first, GLsizei* pcount)
{
  glDrawArrays(mode, first, *pcount);
}

static gboolean
ds_text_ds_renderable_iface_compile(DsRenderable* pself, DsRenderState* state, GCancellable* cancellable, GError** error)
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

  GLuint program =
  ds_render_state_get_current_program(state);

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
 * Perform scheduled deletions
 *
 */

  G_STMT_START
  {
    DsTextList* list;
    DsTextList* next;

    for(list = self->texts;
        list != NULL;
        list = next)
    {
      next = list->next;
      if G_UNLIKELY(list->c->df == TRUE)
      {
        self->texts = (DsTextList*)
        g_list_remove_link(&(self->texts->list_), &(list->list_));
        ds_text_entry_free(list->c);
      }
    }
  }
  G_STMT_END;

/*
 * Compile calls
 *
 */

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

  /* draw things */
  for(list = self->texts;
      list != NULL;
      list = list->next)
  {
    ds_render_state_pcall
    (state,
     G_CALLBACK(glBindVertexArray_s),
     1,
     (guintptr) &(list->c->vao));

    ds_render_state_pcall
    (state,
     G_CALLBACK(glDrawArrays_s),
     3,
     (guintptr) GL_TRIANGLES,
     (guintptr) 0,
     (guintptr) &(list->c->nvt));

    ds_render_state_pcall
    (state,
     G_CALLBACK(glBindVertexArray),
     1,
     (guintptr) 0);
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

static void inline
ds_text_entry_dispose(DsTextEntry* entry)
{
  __gl_try_catch(
    glDeleteVertexArrays(1, &(entry->vao));
    glDeleteBuffers(1, &(entry->vbo));
  ,
    g_critical
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
  );
}

static void
ds_text_entry_free(DsTextEntry* entry)
{
  ds_text_entry_dispose(entry);
  g_slice_free(DsTextEntry, entry);
}

static
void ds_text_class_finalize(GObject* pself) {
  DsText* self = DS_TEXT(pself);
  DsTextList* list;

  for(list = self->texts;
      list != NULL;
      list = list->next)
  {
    ds_text_entry_free(list->c);
    list->c = NULL;
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
              DsTextHandle    text_handle,
              const gchar    *text_,
              vec2            position,
              GCancellable   *cancellable,
              GError        **error)
{
  g_return_val_if_fail(DS_TEXT(text), NULL);
  g_return_val_if_fail(text_ != NULL, NULL);
  g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  DsTextHandle handle = text_handle;
  DsTextEntry* data = NULL;
  GLuint vao = 0, vbo = 0;
  gsize n_vertices = 0;

/*
 * Generate VAO
 *
 */

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

/*
 * Make handle
 *
 */

  if(handle == NULL)
  {
    data =
    g_slice_new(DsTextEntry);

    text->texts =
    (DsTextList*)
    g_list_concat
    (&(text->texts->list_),
     g_list_append
     (NULL,
      (gpointer)
      data));

    handle =
    (DsTextHandle)
    data;
  }
  else
  {
    data =
    (DsTextEntry*)
    handle;

    ds_text_entry_dispose(data);
  }

/*
 * Fill handle
 *
 */

  data->vao = ds_steal_handle_id(&vao);
  data->vbo = ds_steal_handle_id(&vbo);
  data->nvt = n_vertices;
  data->df = FALSE;

  handle = (DsTextHandle) data;
_error_:
  if G_UNLIKELY(success == FALSE)
    handle = NULL;
  if G_UNLIKELY(vao != 0)
    glDeleteVertexArrays(1, &vao);
  if G_UNLIKELY(vbo != 0)
    glDeleteBuffers(1, &vbo);
return handle;
}

void
ds_text_unprint(DsText         *text,
                DsTextHandle    handle)
{
  g_return_if_fail(DS_TEXT(text));

  if(handle != NULL)
  {
    ((DsTextEntry*) handle)->df = TRUE;
  }
}
