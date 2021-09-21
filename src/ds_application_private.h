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
#ifndef __DS_APPLICATION_PRIVATE_INCLUDED__
#define __DS_APPLICATION_PRIVATE_INCLUDED__ 1
#include <ds_application.h>
#include <ds_luaobj.h>
#include <ds_macros.h>
#include <ds_pipeline.h>
#include <ds_saves.h>
#include <ds_settings.h>

#define DS_EVENTS_PUSH "__DS_EVENTS_PUSH"

typedef struct _MvpBuilderData MvpBuilderData;

#if __cplusplus
extern "C" {
#endif // __cplusplus

/* SDL2 */
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <GL/glew.h>

/* CGLM */
#include <cglm/cglm.h>

/* defined in ds_utils.vala */
G_GNUC_INTERNAL
GFile*
_ds_base_data_dir_pick(GCancellable  *cancellable,
                       GError       **error);
G_GNUC_INTERNAL
GFile*
_ds_base_data_dir_child(const gchar   *name,
                        GFile         *basedatadir,
                        GCancellable  *cancellable,
                        GError       **error);

/* defined in ds_application_intr.c */
G_GNUC_INTERNAL
void
_ds_application_init_mvp_builder(DsApplication  *self,
                                 GSettings      *gsettings);
G_GNUC_INTERNAL
void
_ds_application_update_projection(DsApplication  *self,
                                  DsPipeline     *pipeline);
G_GNUC_INTERNAL
void
_ds_application_update_view(DsApplication  *self,
                            DsPipeline     *pipeline,
                            gfloat          x,
                            gfloat          y,
                            gfloat          xrel,
                            gfloat          yrel);

#if __cplusplus
}
#endif // __cplusplus

struct _DsApplication
{
  GApplication parent_instance;

  /*<private>*/
  DsSettings* dssettings;
  GSettings* gsettings;
  GFile* basedatadir;
  GFile* savesdir;
  lua_State* L;
  guint sdl_init;
  guint img_init;
  guint ttf_init;
  SDL_Window* window;
  SDL_GLContext* glctx;
  guint glew_init;
  gint width;
  gint height;
  gint viewport_w;
  gint viewport_h;
  gboolean framelimit;
  DsPipeline* pipeline;

  /*<private>*/
  struct _MvpBuilderData
  {
    gfloat fov;
    vec3 position;
    vec3 worldup;
    vec3 front_;
    gfloat yaw;
    gfloat pitch;
    gfloat sensitivity;
  } mvp_builder;
};

#endif // __DS_APPLICATION_PRIVATE_INCLUDED__
