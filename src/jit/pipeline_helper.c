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

G_GNUC_INTERNAL
void
_ds_jit_helper_update_model(JitMvps* mvps, mat4 model)
{
  glm_mat4_copy(model, mvps->model);
}

G_GNUC_INTERNAL
void
_ds_jit_helper_update_mvps(JitMvps* mvps)
{
  glm_mat4_mul(mvps->projection, mvps->view, mvps->mvp);
  glm_mat4_mul(mvps->mvp, mvps->model, mvps->mvp);
}
