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
#include <jit.h>

#define DEBUG_MVP (0)

#if DEBUG_MVP
static void
debug_mvps(JitMvps* mvps)
{
# define F "%02.08f "
# define v mvps->model

  g_print
  ("model:\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n",
   v[0][0], v[0][1], v[0][2], v[0][3],
   v[1][0], v[1][1], v[1][2], v[1][3],
   v[2][0], v[2][1], v[2][2], v[2][3],
   v[3][0], v[3][1], v[3][2], v[3][3]);

# undef v
# define v mvps->view

  g_print
  ("view:\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n",
   v[0][0], v[0][1], v[0][2], v[0][3],
   v[1][0], v[1][1], v[1][2], v[1][3],
   v[2][0], v[2][1], v[2][2], v[2][3],
   v[3][0], v[3][1], v[3][2], v[3][3]);

# undef v
# define v mvps->projection

  g_print
  ("projection:\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n",
   v[0][0], v[0][1], v[0][2], v[0][3],
   v[1][0], v[1][1], v[1][2], v[1][3],
   v[2][0], v[2][1], v[2][2], v[2][3],
   v[3][0], v[3][1], v[3][2], v[3][3]);

# undef v
# define v mvps->jvp

  g_print
  ("jvp:\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n",
   v[0][0], v[0][1], v[0][2], v[0][3],
   v[1][0], v[1][1], v[1][2], v[1][3],
   v[2][0], v[2][1], v[2][2], v[2][3],
   v[3][0], v[3][1], v[3][2], v[3][3]);

# undef v
# define v mvps->mvp

  g_print
  ("mvp:\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n"
   F F F F "\r\n",
   v[0][0], v[0][1], v[0][2], v[0][3],
   v[1][0], v[1][1], v[1][2], v[1][3],
   v[2][0], v[2][1], v[2][2], v[2][3],
   v[3][0], v[3][1], v[3][2], v[3][3]);

# undef F
# undef v
}
#endif // DEBUG_MVP

G_GNUC_INTERNAL
void
_ds_jit_helper_update_model(JitMvps* mvps, mat4 model)
{
  glm_mat4_copy(model, mvps->model);
}

G_GNUC_INTERNAL
void
_ds_jit_helper_update_jvp(JitMvps* mvps)
{
  glm_mat4_mul(mvps->projection, mvps->view, mvps->jvp);
#if DEBUG_MVP
  debug_mvps(mvps);
#endif // DEBUG_MVP
}

G_GNUC_INTERNAL
void
_ds_jit_helper_update_mvp(JitMvps* mvps)
{
  glm_mat4_mul(mvps->jvp, mvps->model, mvps->mvp);
#if DEBUG_MVP
  debug_mvps(mvps);
#endif // DEBUG_MVP
}

G_GNUC_INTERNAL
void
_ds_jit_helper_update_mvps(JitMvps* mvps)
{
  glm_mat4_mul(mvps->projection, mvps->view, mvps->jvp);
  glm_mat4_mul(mvps->jvp, mvps->model, mvps->mvp);
#if DEBUG_MVP
  debug_mvps(mvps);
#endif // DEBUG_MVP
}
