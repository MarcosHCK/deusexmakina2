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
#ifndef __DS_RENDERABLE_INCLUDED__
#define __DS_RENDERABLE_INCLUDED__ 1
#include <ds_export.h>
#include <ds_gl.h>
#include <gio/gio.h>

#define DS_TYPE_RENDERABLE            (ds_renderable_get_type())
#define DS_RENDERABLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DS_TYPE_RENDERABLE, DsRenderable))
#define DS_RENDERABLE_CLASS(obj)      (G_TYPE_CHECK_CLASS_CAST((obj), DS_TYPE_RENDERABLE, DsRenderableIface))
#define DS_IS_RENDERABLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), DS_TYPE_RENDERABLE))
#define DS_RENDERABLE_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), DS_TYPE_RENDERABLE, DsRenderableIface))

typedef struct _DsRenderable        DsRenderable;
typedef struct _DsRenderableIface   DsRenderableIface;
typedef struct _DsRenderState       DsRenderState;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_renderable_get_type();

/**
 * _DsRenderableIface:
 * @parent_iface: parent type data.
 * @compile: compiles every code needed to render this object.
 *
 * The #DsRenderable defined rules to render objects.
 */
struct _DsRenderableIface
{
  GTypeInterface parent_iface;
  gboolean (*compile) (DsRenderable* renderable, DsRenderState* state, GCancellable* cancellable, GError** error);
};

DEUSEXMAKINA2_API
gboolean
ds_renderable_compile(DsRenderable   *renderable,
                      DsRenderState  *state,
                      GCancellable   *cancellable,
                      GError        **error);

GLuint
ds_render_state_get_current_program(DsRenderState* state);
void
ds_render_state_switch_vertex_array(DsRenderState* state, GLuint vao);
void
ds_render_state_call(DsRenderState  *state,
                     GCallback       callback,
                     guint           n_params,
                     ...);
void
ds_render_state_pcall(DsRenderState  *state,
                      GCallback       callback,
                      guint           n_params,
                      ...);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_RENDERABLE_INCLUDED__
