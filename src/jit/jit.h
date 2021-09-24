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
#ifndef __DS_JIT_INCLUDED__
#define __DS_JIT_INCLUDED__
#include <cglm/cglm.h>
#include <src/ds_gl.h>

#ifdef __INSIDE_DYNASM_FILE__
# ifdef G_OS_WINDOWS
#   include <windows.h>
# else
#   include <sys/mman.h>
#   if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#     define MAP_ANONYMOUS MAP_ANON
#   endif
# endif // G_OS_WINDOWS
# define Dst        ((dasm_State**)&(ctx->pd))
#else // __INSIDE_DYNASM_FILE__
# define jitmain         ((JitMain)(ctx->labels[ctx->n_main]))
#endif // __INSIDE_DYNASM_FILE__

typedef struct {
/*
 * IMPORTANT
 * keep 'mvp' on top since
 * compiled code uses structure
 * pointer as 'mvp' address
 *
 */
  mat4 mvp;
  mat4 projection;
  mat4 view;
  mat4 model;
} JitMvps;

typedef struct {
  gpointer pd;
  gpointer* labels;
  guint n_labels;
  guint n_main;
  gpointer block;
  gsize blocksz;
  JitMvps mvps;
  GLuint mvpl;
} JitState;

typedef void (*JitMain) (gpointer instance, JitMvps* mvps, GError** error);

#if __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Public API
 *
 */

G_GNUC_INTERNAL
void
_ds_jit_compile_start(JitState* ctx);
G_GNUC_INTERNAL
void
_ds_jit_compile_end(JitState* ctx);
G_GNUC_INTERNAL
void
_ds_jit_compile_free(JitState* ctx);

G_GNUC_INTERNAL
void
_ds_jit_compile_call_valist(JitState *ctx,
                            GCallback callback,
                            gboolean  protected_,
                            guint     n_params,
                            va_list   l);
G_GNUC_INTERNAL
void
_ds_jit_compile_call(JitState  *ctx,
                     GCallback  callback,
                     gboolean  protected_,
                     guint      n_params,
                     ...);
G_GNUC_INTERNAL
void
_ds_jit_compile_mvp_model(JitState *ctx,
                          mat4      model);

/*
 * Internal API
 *
 */

G_GNUC_INTERNAL
void
_ds_jit_helper_update_model(JitMvps* mvps,
                            mat4 model);
G_GNUC_INTERNAL
void
_ds_jit_helper_update_mvps(JitMvps* mvps);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_JIT_INCLUDED__
