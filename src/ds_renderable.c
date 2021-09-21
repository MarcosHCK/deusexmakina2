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

static
void ds_renderable_default_init(DsRenderableIface* iface);

/*
 * Interface definition
 *
 */

GType
ds_renderable_get_type (void)
{
  static
  GType renderable_type = 0;

  if G_UNLIKELY(renderable_type == 0)
  {
    const
    GTypeInfo type_info = {
      sizeof(DsRenderableIface),
      NULL,
      NULL,
      (GClassInitFunc)
      ds_renderable_default_init,
      NULL,
      NULL,
      0,
      0,
      NULL
    };

    renderable_type =
    g_type_register_static
    (G_TYPE_INTERFACE,
     "DsRenderable",
     &type_info,
     0);
  }
return renderable_type;
}

static gboolean
ds_renderable_default_compile(DsRenderable         *renderable,
                              DsRenderState        *state,
                              GLuint                program,
                              GCancellable         *cancellable,
                              GError              **error)
{
  g_warning
  ("DsRenderable::compile not implemented for '%s'\r\n",
   g_type_name(G_TYPE_FROM_INSTANCE(renderable)));
return FALSE;
}

static
void ds_renderable_default_init(DsRenderableIface* iface) {
  iface->compile = ds_renderable_default_compile;
}

/*
 * Object methods
 *
 */

gboolean
ds_renderable_compile(DsRenderable         *renderable,
                      DsRenderState        *state,
                      GLuint                program,
                      GCancellable         *cancellable,
                      GError              **error)
{
  g_return_val_if_fail(DS_IS_RENDERABLE(renderable), FALSE);
  g_return_val_if_fail(state != NULL, FALSE);
  g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), FALSE);
  g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

  DsRenderableIface* iface =
  DS_RENDERABLE_GET_IFACE(renderable);
return iface->compile(renderable, state, program, cancellable, error);
}

void
ds_render_state_pcall(DsRenderState  *state,
                      GCallback       callback,
                      guint           n_params,
                      ...)
{
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

void
ds_render_state_call(DsRenderState  *state,
                     GCallback       callback,
                     guint           n_params,
                     ...)
{
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
ds_render_state_mvp_set_model(DsRenderState  *state,
                              gfloat*         mvp_model)
{
  JitState* ctx = (JitState*) state;
  _ds_jit_compile_mvp_model(ctx, (gpointer) mvp_model);
}
