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
#include <ds_model_private.h>

static const
goffset format_offsets[] =
{
  G_STRUCT_OFFSET(DsModelVertex, position),
  G_STRUCT_OFFSET(DsModelVertex, normal),
  G_STRUCT_OFFSET(DsModelVertex, uvw),
  G_STRUCT_OFFSET(DsModelVertex, tangent),
  G_STRUCT_OFFSET(DsModelVertex, bitangent),
};

static const
guint format_floats[] =
{
  3, 3, 3, 3, 3,
};

#define n_attribs 5
G_STATIC_ASSERT(G_N_ELEMENTS(format_offsets) >= n_attribs);
G_STATIC_ASSERT(G_N_ELEMENTS(format_floats) >= n_attribs);

G_GNUC_INTERNAL
gboolean
_ds_model_make_vao(DsModel* self, GError** error)
{
  gboolean success = TRUE;
  guint i;

  DsModelClass* klass = DS_MODEL_GET_CLASS(self);
  if G_UNLIKELY(klass->vao == 0)
  {
    /* generate vertex array */
    __gl_try_catch(
      glGenVertexArrays(1, &(klass->vao));
      glBindVertexArray(klass->vao);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    g_assert(klass->vao > 0);

#if GL_VERSION_4_3 == 1
    for(i = 0;
        i < n_attribs;
        i++)
    {
      __gl_try_catch(
        glEnableVertexAttribArray(i);
        glVertexAttribFormat(i, format_floats[i], GL_FLOAT, GL_FALSE, format_offsets[i]);
        glVertexAttribBinding(i, 0);
      ,
        g_propagate_error(error, glerror);
        goto_error();
      );
    }
#else
    for(i = 0;
        i < n_attribs;
        i++)
    {
      __gl_try_catch(
        glEnableVertexAttribArray(i);
      ,
        g_propagate_error(error, glerror);
        goto_error();
      );
    }
#endif // GL_VERSION_4_3
    __gl_try_catch(
      glBindVertexArray(0);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }
_error_:
return success;
}

G_GNUC_INTERNAL
gboolean
_ds_model_compile_switch_vbo(DsModel* self, DsRenderState* state, GError** error)
{
  gboolean success = TRUE;
  guint i;

#if GL_VERSION_4_3 == 1

  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindVertexBuffer),
   4,
   (guintptr) 0,
   (guintptr) self->vbo,
   (guintptr) 0,
   (guintptr) sizeof(DsModelVertex));

#else

  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindBuffer),
   2,
   (guintptr) GL_ARRAY_BUFFER,
   (guintptr) self->vbo);

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
_error_:
return success;
}

G_GNUC_INTERNAL
gboolean
_ds_model_compile_switch_ibo(DsModel* self, DsRenderState* state, GError** error)
{
  gboolean success = TRUE;

  ds_render_state_pcall
  (state,
   G_CALLBACK(glBindBuffer),
   2,
   (guintptr) GL_ELEMENT_ARRAY_BUFFER,
   (guintptr) self->ibo);
_error_:
return success;
}
