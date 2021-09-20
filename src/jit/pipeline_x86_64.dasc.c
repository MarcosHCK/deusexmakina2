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
#include <overwrites.h>
#include <dynasm/dasm_proto.h>
#include <dynasm/dasm_x86.h>
#include <jit.h>

|.arch x64
|.section code
|.globals globl_
|.actionlist actions
|.globalnames globl_names
|.externnames extern_names

|.if fastcall_style == "linux"
|.define arg1, rdi
|.define arg2, rsi
|.define arg3, rdx
|.define arg4, rcx
|.define arg5, r8
|.define arg6, r9
|.endif
|.if fastcall_style == "windows"
|.define arg1, rcx
|.define arg2, rdx
|.define arg3, r8
|.define arg4, r9
|.define arg5, [rsp+0*8]
|.define arg5, [rsp+1*8]
|.endif

|.macro epilogue
| pop rbx
| ret
|.endmacro

|.macro invoke, name
| mov64 rax, ((guintptr) name)
| call rax
|.endmacro

/*
 * if G_UNLIKELY
 *  (ds_gl_has_error() == TRUE)
 *  {
 *    *error = ds_gl_get_error();
 *    return;
 *  }
 *
 */
|.macro __gl_catch
| invoke ds_gl_has_error
| test rax, rax
| jz >1
| invoke ds_gl_get_error
| mov [rbx], rax
| epilogue
|1:
|.endmacro

void
_ds_jit_compile_start(JitState* ctx)
{
/*
 * Pre-init dasm state
 *
 */

  dasm_init(Dst, DASM_MAXSECTION);

/*
 * Setup globals
 *
 */

  ctx->labels = g_slice_alloc0(sizeof(gpointer) * globl__MAX);
  ctx->n_labels = globl__MAX;
  ctx->n_main = globl_jitmain;

  dasm_setupglobal(Dst, ctx->labels, globl__MAX);

/*
 * Finish setup
 *
 */

  dasm_setup(Dst, actions);

/*
 * Setup dynamic labels
 *
 */

  unsigned npc = 0;
  dasm_growpc(Dst, npc);

/*
 * Put prologue
 *
 */

  |.code
  |->jitmain:

  /* GError** -> rbx */
  | push rbx
  | mov rbx, arg1
}

void
_ds_jit_compile_end(JitState* ctx)
{
/*
 * Put epilogue
 *
 */

  | epilogue

/*
 * Link
 *
 */

  size_t sz;
  gpointer buf;

  dasm_link(Dst, &sz);
#ifdef G_OS_WINDOWS
  buf = VirtualAlloc(0, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
  buf = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // G_OS_WINDOWS

/*
 * Actually produce
 * machine code
 *
 */

  dasm_encode(Dst, buf);
#ifdef G_OS_WINDOWS
  G_STMT_START {
    DWORD dwOld;
    VirtualProtect(buf, sz, PAGE_EXECUTE_READ, &dwOld);
  } G_STMT_END;
#else
  mprotect(buf, sz, PROT_READ | PROT_EXEC);
#endif // G_OS_WINDOWS

/*
 * Finish process
 *
 */

  ctx->block = buf;
  ctx->blocksz = sz;
  dasm_free(Dst);
}

void
_ds_jit_compile_free(JitState* ctx)
{
  if G_LIKELY(ctx->labels != NULL)
  {
    g_slice_free1
    (sizeof(gpointer) * ctx->n_labels,
     ctx->labels);
    ctx->labels = NULL;
    ctx->n_labels = 0;
  }

  if G_UNLIKELY(ctx->block != NULL)
  {
    munmap
    (ctx->block,
     ctx->blocksz);
    ctx->block = NULL;
    ctx->blocksz = 0;
  }
}

void
_ds_jit_compile_use_shader(JitState* ctx,
                           GLuint shader)
{
  | mov64 arg1, ((guintptr) shader)
  | invoke glUseProgram
  | __gl_catch
}
