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
#include <ds_font.h>
#include <ds_luaobj.h>
#include <ds_macros.h>
#include <ds_pipeline.h>
#include <ds_saves.h>
#include <ds_settings.h>
#include <ds_text.h>

#define DS_EVENTS_PUSH "__DS_EVENTS_PUSH"

typedef struct _RendererData RendererData;

#if __cplusplus
extern "C" {
#endif // __cplusplus

/* SDL2 */
#include <SDL.h>
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
_ds_base_cache_dir_pick(GCancellable  *cancellable,
                        GError       **error);
G_GNUC_INTERNAL
GFile*
_ds_base_dirs_child(const gchar   *name,
                    GFile         *basedatadir,
                    GCancellable  *cancellable,
                    GError       **error);

/* defined in ds_renderer_data.c */
G_GNUC_INTERNAL
void
_ds_renderer_data_init(DsApplication  *self,
                       GSettings      *gsettings);
G_GNUC_INTERNAL
void
_ds_renderer_data_set_projection(DsApplication *self,
                                 DsPipeline    *pipeline);
G_GNUC_INTERNAL
void
_ds_renderer_data_set_view(DsApplication *self,
                           DsPipeline    *pipeline,
                           gfloat         x,
                           gfloat         y,
                           gfloat         xrel,
                           gfloat         yrel);

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
  GFile* basecachedir;
  GFile* savesdir;
  GFile* glcachedir;
  lua_State* L;
  guint sdl_init;
  SDL_Window* window;
  SDL_GLContext* glctx;
  guint glew_init;
  DsPipeline* pipeline;
  DsFont* debug_font;
  DsText* debug_text;

  /*<private>*/
  struct _RendererData
  {
    gfloat fov;
    gint width;
    gint height;
    gint viewport_w;
    gint viewport_h;
    gboolean framelimit;

    vec3 position;
    vec3 worldup;
    vec3 front_;
    gfloat yaw;
    gfloat pitch;
    gfloat sensitivity;
  } renderer_data;
};

#endif // __DS_APPLICATION_PRIVATE_INCLUDED__
