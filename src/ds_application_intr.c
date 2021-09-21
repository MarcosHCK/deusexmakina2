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

G_GNUC_INTERNAL
void
_ds_application_init_mvp_builder(DsApplication  *self,
                                 GSettings      *gsettings)
{
  MvpBuilderData* d =
  &(self->mvp_builder);

  glm_vec3_fill(d->position, 0.f);
  glm_vec3_fill(d->worldup, 0.f);
  glm_vec3_fill(d->front_, 0.f);

  d->worldup[1] = 1.f;
  d->front_[2] = -1.f;
  d->yaw = -90.f;
  d->pitch = 0.f;

  gdouble fov;
  g_settings_get
  (gsettings,
   "fov", "d",
   &fov);

  d->fov = (gfloat) fov;
}

G_GNUC_INTERNAL
void
_ds_application_update_projection(DsApplication  *self,
                                  DsPipeline     *pipeline)
{
  MvpBuilderData* d =
  &(self->mvp_builder);

  mat4 projection;
  glm_perspective
  (d->fov,
     ((gfloat) self->width)
   / ((gfloat) self->height),
   0.1f,
   100.0f,
   projection);

  ds_pipeline_mvps_set_projection(pipeline, (gfloat*) projection);
}

#define position    d->position
#define worldup     d->worldup
#define front       d->front_
#define yaw         d->yaw
#define pitch       d->pitch
#define sensitivity d->sensitivity

G_GNUC_INTERNAL
void
_ds_application_update_view(DsApplication  *self,
                            DsPipeline     *pipeline,
                            gfloat          x,
                            gfloat          y,
                            gfloat          xrel,
                            gfloat          yrel)
{
  MvpBuilderData* d =
  &(self->mvp_builder);

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

  ds_pipeline_mvps_set_view(pipeline, (gfloat*) camera);
}

#undef sensitivity
#undef pitch
#undef yaw
#undef front
#undef worldup
#undef position
