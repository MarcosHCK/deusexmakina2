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
# define jitmain         ((GFunc)(ctx->labels[ctx->n_main]))
#endif // __INSIDE_DYNASM_FILE__

typedef struct {
  gpointer pd;
  gpointer* labels;
  guint n_labels;
  guint n_main;
  gpointer block;
  gsize blocksz;
} JitState;

#if __cplusplus
extern "C" {
#endif // __cplusplus

void
_ds_jit_compile_start(JitState* ctx);
void
_ds_jit_compile_end(JitState* ctx);
void
_ds_jit_compile_free(JitState* ctx);
void
_ds_jit_compile_use_shader(JitState* ctx,
                           GLuint shader);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_JIT_INCLUDED__
