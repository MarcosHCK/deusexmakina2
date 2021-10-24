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
#include <ds_font.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_pencil.h>
#include <ds_renderable.h>
#include <ds_text.h>

G_DEFINE_QUARK(ds-text-error-quark,
               ds_text_error);

static void
ds_text_ds_renderable_iface_init(DsRenderableIface* iface);
static
mat4 scale;

typedef struct _DsTextEntry DsTextEntry;

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
  DsPencil* pencil;
  GList* texts;
};

struct _DsTextClass
{
  GObjectClass parent_class;
};

struct _DsTextEntry
{
  GLuint vbo;
  gsize  nvt;
};

enum {
  prop_0,
  prop_provider,
  prop_pencil,
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
 (DS_TYPE_RENDERABLE,
  ds_text_ds_renderable_iface_init)
 {
   glm_mat4_identity(scale);
 });

static gboolean
ds_text_ds_renderable_iface_compile(DsRenderable* pself, DsRenderState* state, GCancellable* cancellable, GError** error)
{
  DsText* self = DS_TEXT(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;
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

_error_:
  __gl_try_catch(
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
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
  case prop_pencil:
    g_set_object(&(self->pencil), g_value_get_object(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_text_class_constructed(GObject* pself) {
  DsText* self = DS_TEXT(pself);

  G_OBJECT_CLASS(ds_text_parent_class)->constructed(pself);

  g_object_get(self->font, "texture-id", &(self->tio), NULL);

  if(self->pencil == NULL)
  {
    self->pencil =
    ds_pencil_get_default();
    g_assert(self->pencil != NULL);
  }
}

static void inline
ds_text_entry_dispose(DsTextEntry* entry)
{
  __gl_try_catch(
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
  GList* list;

  for(list = self->texts;
      list != NULL;
      list = list->next)
  {
    g_clear_pointer(&(list->data), ds_text_entry_free);
  }
G_OBJECT_CLASS(ds_text_parent_class)->finalize(pself);
}

static
void ds_text_class_dispose(GObject* pself) {
  DsText* self = DS_TEXT(pself);
  g_clear_object(&(self->font));
  g_clear_object(&(self->pencil));
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

  properties[prop_pencil] =
    g_param_spec_object
    (_TRIPLET("pencil"),
     DS_TYPE_PENCIL,
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
_ds_font_puts(DsFont         *font,
              DsPencil       *pencil,
              const gchar    *text,
              gfloat          x,
              gfloat          y,
              GLuint         *pvbo,
              gsize          *pnvt,
              GError        **error);

/**
 * ds_text_print:
 * @text: an #DsText instance.
 * @text_handle: a text fragment handler.
 * @text_: text to draw.
 * @x: X-axis coordinates where to put @text_.
 * @y: Y-axis coordinates where to put @text_.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Prints a rectangle containing @text_ glyphs.
 * If @text_handle refers to a previously printed
 * text, previous one is replaced by this call.
 * Note: text are printed in a 800x600 pixels screen
 * space, starting for top-left corner.
 *
 * Returns: (transfer none): a handler to text fragment
 * produced by this call. It belongs to @text, so don't
 * mess with it.
 */
DsTextHandle
ds_text_print(DsText         *text,
              DsTextHandle    text_handle,
              const gchar    *text_,
              gfloat          x,
              gfloat          y,
              GCancellable   *cancellable,
              GError        **error)
{
  g_return_val_if_fail(DS_TEXT(text), NULL);
  g_return_val_if_fail(text_ != NULL, NULL);
  g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
  g_return_val_if_fail(error == NULL || *error == NULL, NULL);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

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

_error_:
return NULL;
}

/**
 * ds_text_unprint:
 * @text: an #DsText instance.
 * @text_handle: a text fragment handler.
 *
 * Erases anything printed by a previous call
 * to #ds_text_print().
 * Note that this function doesn't actually erases
 * anything, it just queues it to be skipped and erased
 * when next frame is to be drown.
 */
void
ds_text_unprint(DsText         *text,
                DsTextHandle    text_handle)
{
  g_return_if_fail(DS_TEXT(text));
  g_return_if_fail(text_handle != NULL);
}
