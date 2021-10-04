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
#include <ds_renderable.h>
#include <jit/jit.h>

typedef DsRenderableIface DsRenderableInterface;

static
void ds_renderable_default_init(DsRenderableIface* iface);

/*
 * Interface definition
 *
 */

G_DEFINE_INTERFACE
(DsRenderable,
 ds_renderable,
 G_TYPE_OBJECT);

static gboolean
ds_renderable_default_compile(DsRenderable   *renderable,
                              DsRenderState  *state,
                              GCancellable   *cancellable,
                              GError        **error)
{
  g_warning
  ("DsRenderable::compile not implemented for '%s'\r\n",
   g_type_name(G_TYPE_FROM_INSTANCE(renderable)));
return FALSE;
}

static void
ds_renderable_default_query_mvp_step(DsRenderable* renderable, DsRenderState* state)
{
}

static void
ds_renderable_default_query_mvp_reset(DsRenderable* renderable)
{
}

static
void ds_renderable_default_init(DsRenderableIface* iface) {
  iface->compile = ds_renderable_default_compile;
  iface->query_mvp_step = ds_renderable_default_query_mvp_step;
  iface->query_mvp_reset = ds_renderable_default_query_mvp_reset;
}

/*
 * Object methods
 *
 */

gboolean
ds_renderable_compile(DsRenderable         *renderable,
                      DsRenderState        *state,
                      GCancellable         *cancellable,
                      GError              **error)
{
  g_return_val_if_fail(DS_IS_RENDERABLE(renderable), FALSE);
  g_return_val_if_fail(state != NULL, FALSE);
  g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

  DsRenderableIface* iface =
  DS_RENDERABLE_GET_IFACE(renderable);
return iface->compile(renderable, state, cancellable, error);
}

GLuint
ds_render_state_get_current_program(DsRenderState* state)
{
  g_return_val_if_fail(state != NULL, 0);
return ((JitState*) state)->pid;
}

void
ds_render_state_call(DsRenderState  *state,
                     GCallback       callback,
                     guint           n_params,
                     ...)
{
  g_return_if_fail(state != NULL);
  g_return_if_fail(callback != NULL);
  g_return_if_fail(n_params > 0);

  va_list l;
  va_start(l, n_params);

  _ds_jit_compile_call_valist
  ((JitState*)
   state,
   callback,
   FALSE,
   n_params,
   l);

  va_end(l);
}

void
ds_render_state_pcall(DsRenderState  *state,
                      GCallback       callback,
                      guint           n_params,
                      ...)
{
  g_return_if_fail(state != NULL);
  g_return_if_fail(callback != NULL);
  g_return_if_fail(n_params > 0);

  va_list l;
  va_start(l, n_params);

  _ds_jit_compile_call_valist
  ((JitState*)
   state,
   callback,
   TRUE,
   n_params,
   l);

  va_end(l);
}
