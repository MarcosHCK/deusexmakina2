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
#include <ds_application_private.h>
#include <ds_mvpholder.h>

G_GNUC_INTERNAL
void
_ds_renderer_data_init(DsApplication  *self,
                       GSettings      *gsettings)
{
  RendererData* d =
  &(self->renderer_data);

  glm_vec3_fill(d->position, 0.f);
  glm_vec3_fill(d->worldup, 0.f);
  glm_vec3_fill(d->front_, 0.f);

  d->worldup[1] = 1.f;
  d->front_[2] = -1.f;
  d->yaw = -90.f;
  d->pitch = 0.f;

  gdouble fov, sensitivity;
  g_settings_get(gsettings, "fov", "d", &fov);
  g_settings_get(gsettings, "sensitivity", "d", &sensitivity);
  g_settings_get(gsettings, "framelimit", "b", &(d->framelimit));

  d->fov = (gfloat) fov;
  d->sensitivity = (gfloat) sensitivity;
}

G_GNUC_INTERNAL
void
_ds_renderer_data_set_projection(DsApplication *self,
                                 DsPipeline    *pipeline)
{
  RendererData* d =
  &(self->renderer_data);

  mat4 projection;
  glm_perspective
  (glm_rad(d->fov),
     ((gfloat) d->width)
   / ((gfloat) d->height),
   0.1f,
   100.0f,
   projection);

  ds_mvp_holder_set_projection(DS_MVP_HOLDER(pipeline), (gfloat*) projection);
}

#define position    d->position
#define worldup     d->worldup
#define front       d->front_
#define yaw         d->yaw
#define pitch       d->pitch
#define sensitivity d->sensitivity

G_GNUC_INTERNAL
void
_ds_renderer_data_set_view(DsApplication *self,
                           DsPipeline    *pipeline,
                           gfloat         x,
                           gfloat         y,
                           gfloat         xrel,
                           gfloat         yrel)
{
  RendererData* d =
  &(self->renderer_data);

  yaw = yaw + (xrel * sensitivity);
  pitch = pitch - (yrel * sensitivity);

  if(pitch > 89.f)
    pitch = 89.f;
  else
  if(pitch < -89.f)
    pitch = -89.f;

  gfloat yaw_r = glm_rad(yaw);
  gfloat pitch_r = glm_rad(pitch);

  front[0] = cos(yaw_r) * cos(pitch_r);
  front[1] = sin(pitch_r);
  front[2] = sin(yaw_r) * cos(pitch_r);

  vec3 right;
  vec3 up;
  vec3 center;
  mat4 camera;

  glm_vec3_cross(front, worldup, right);
  glm_vec3_cross(right, front, up);
  glm_vec3_add(position, front, center);
  glm_lookat(position, center, up, camera);

  ds_mvp_holder_set_view(DS_MVP_HOLDER(pipeline), (gfloat*) camera);
}

#undef sensitivity
#undef pitch
#undef yaw
#undef front
#undef worldup
#undef position
