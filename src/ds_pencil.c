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
#include <ds_macros.h>
#include <ds_pencil.h>
#include <gio/gio.h>

G_DEFINE_QUARK(ds-pencil-error-quark,
               ds_pencil_error);

static void
ds_pencil_g_initable_iface_init(GInitableIface* iface);
static
GWeakRef __default__ = {0};

/*
 * Vertex array specs
 *
 */

static const
goffset format_offsets[] =
{
  G_STRUCT_OFFSET(DsPencilVertex, position),
  G_STRUCT_OFFSET(DsPencilVertex, normal),
  G_STRUCT_OFFSET(DsPencilVertex, uvw),
  G_STRUCT_OFFSET(DsPencilVertex, tangent),
  G_STRUCT_OFFSET(DsPencilVertex, bitangent),
};

static const
guint format_floats[] =
{
  3,
  3,
  3,
  3,
  3,
};

#define n_attribs (5)

/* sanity checks */
G_STATIC_ASSERT(G_N_ELEMENTS(format_offsets) == n_attribs);
G_STATIC_ASSERT(G_N_ELEMENTS(format_floats) == n_attribs);

/*
 * Definition
 *
 */

struct _DsPencil
{
  GObject parent_instance;

  /*<private>*/
  GLuint vao;
};

struct _DsPencilClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE_WITH_CODE
(DsPencil,
 ds_pencil,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_pencil_g_initable_iface_init));

static gboolean
ds_pencil_g_initable_iface_init_sync(GInitable* pself, GCancellable* cancellable, GError** error)
{
  DsPencil* self = DS_PENCIL(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  guint i;

  DsPencil* default_ =
  ds_pencil_get_default();
  if(G_IS_OBJECT(default_))
  {
    g_set_error_literal
    (error,
     DS_PENCIL_ERROR,
     DS_PENCIL_ERROR_CREATED,
     "There is an instance of this object already, use ds_pencil_get_default()\r\n");
    g_object_unref(default_);
    goto_error();
  }

  __gl_try_catch(
    glGenVertexArrays(1, &(self->vao));
    glBindVertexArray(self->vao);
    g_assert(self->vao > 0);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );


  for(i = 0;
      i < n_attribs;
      i++)
  {
    __gl_try_catch(
      glEnableVertexAttribArray(i);
#if GL_VERSION_4_3 == 1
      glVertexAttribFormat(i, format_floats[i], GL_FLOAT, GL_FALSE, format_offsets[i]);
      glVertexAttribBinding(i, 0);
#endif // GL_VERSION_4_3
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

  g_weak_ref_set(&__default__, self);
_error_:
return success;
}

static void
ds_pencil_g_initable_iface_init(GInitableIface* iface)
{
  iface->init = ds_pencil_g_initable_iface_init_sync;
}

static void
ds_pencil_class_finalize(GObject* pself)
{
  DsPencil* self = DS_PENCIL(pself);
  if G_LIKELY(self->vao != 0)
  {
    __gl_try_catch(
      glDeleteVertexArrays(1, &(self->vao));
    ,
      g_warning
      ("(%s: %i): %s: %i: %s\r\n",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string(glerror->domain),
       glerror->code,
       glerror->message);
      g_error_free(glerror);
    );
  }
G_OBJECT_CLASS(ds_pencil_parent_class)->finalize(pself);
}

static void
ds_pencil_class_dispose(GObject* pself)
{
  DsPencil* self = DS_PENCIL(pself);
G_OBJECT_CLASS(ds_pencil_parent_class)->dispose(pself);
}

static void
ds_pencil_class_init(DsPencilClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  oclass->finalize = ds_pencil_class_finalize;
  oclass->dispose = ds_pencil_class_dispose;
}

static void
ds_pencil_init(DsPencil* self)
{
}

/*
 *
 *
 */

/**
 * ds_pencil_new: (constructor)
 * @error: return location for a #GError.
 *
 * Returns a new instance of #DsPencil object.
 * Also set process-default pencil if there aren't
 * one, otherwise returns an error.
 *
 * Returns: (transfer full): See description.
 */
DsPencil*
ds_pencil_new(GError** error)
{
  return (DsPencil*)
  g_initable_new
  (DS_TYPE_PENCIL,
   NULL,
   error,
   NULL);
}

/**
 * ds_pencil_get_default: (method)
 *
 * Returns process-default pencil.
 *
 * Returns: (transfer full): See description.
 */
DsPencil*
ds_pencil_get_default()
{
return g_weak_ref_get(&__default__);
}

static void
glBindVertexBuffer_s(GLuint bindingindex, GLuint* p_vbo, GLintptr offset, GLsizei stride)
{
  glBindVertexBuffer(bindingindex, *p_vbo, offset, stride);
}

static void
glBindBuffer_s(GLenum target, GLuint* p_vbo)
{
  glBindBuffer(target, *p_vbo);
}

/**
 * ds_pencil_bind: (method)
 * @pencil: a #DsPencil instance (NULL to use process default pencil).
 * @error: return location for a #GError.
 *
 * Immediately bind internal VAO.
 *
 */
gboolean
ds_pencil_bind(DsPencil* pencil, GError** error)
{
  g_return_val_if_fail(DS_IS_PENCIL(pencil), FALSE);
  DsPencil* self = pencil;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  __gl_try_catch(
    glBindVertexArray(self->vao);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

_error_:
return success;
}

/**
 * ds_pencil_switch: (method)
 * @pencil: a #DsPencil instance (NULL to use process default pencil).
 * @state: renderer state over which compile VAO switch.
 * @p_vbo: a pointer to vertex buffer object name.
 *
 * Compiles a VAO switch on @state (if necessary).
 *
 */
void
ds_pencil_switch(DsPencil* pencil, DsRenderState* state, GLuint* p_vbo)
{
  g_return_if_fail(DS_IS_PENCIL(pencil));
  g_return_if_fail(state != NULL);
  g_return_if_fail(p_vbo != NULL && *p_vbo > 0);
  DsPencil* self = pencil;

/*
 * Switch vao
 *
 */

  ds_render_state_switch_vertex_array(state, self->vao);

/*
 * Switch vbo
 *
 */
#if GL_VERSION_4_3 == 1
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexBuffer_s),
   4,
   (guintptr) 0,
   (guintptr) p_vbo,
   (guintptr) 0,
   (guintptr) sizeof(DsPencilVertex));
#else
  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindBuffer_s),
   2,
   (guintptr) GL_ARRAY_BUFFER,
   (guintptr) p_vbo);

  guint i;
  for(i = 0;
      i < n_attribs;
      i++)
  ds_render_state_pcall
  (state,
   G_CALLBACK(glVertexAttribPointer),
   6,
   (guintptr) i,
   (guintptr) format_floats[i],
   (guintptr) GL_FLOAT,
   (guintptr) GL_FALSE,
   (guintptr) sizeof(DsModelVertex),
   (guintptr) format_offsets[i]);
#endif // GL_VERSION_4_3
}
