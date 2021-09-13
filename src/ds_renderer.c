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
#include <ds_renderer.h>

gboolean
_ds_renderer_step(DsApplication* self)
{
  glViewport(0, 0, self->width, self->height);
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
 * Frame timing
 *
 */
  static GLfloat deltaTime = 0.f;
  static GLfloat frameTime = 0.f;

  deltaTime = SDL_GetTicks() - frameTime;
  frameTime = SDL_GetTicks();

/*
 * Present display
 *
 */
  SDL_GL_SwapWindow(self->window);
  if(self->framelimit == TRUE)
    SDL_Delay(10);
return G_SOURCE_CONTINUE;
}

/*
 * init / fini
 *
 */

static void
on_width_changed(GSettings     *gsettings,
                 const gchar   *key,
                 DsApplication *self)
{
  g_settings_get
  (gsettings,
   key, "i",
   &(self->width));
}

static void
on_height_changed(GSettings     *gsettings,
                  const gchar   *key,
                  DsApplication *self)
{
  g_settings_get
  (gsettings,
   key, "i",
   &(self->height));
}

static void
on_framelimit_changed(GSettings     *gsettings,
                      const gchar   *key,
                      DsApplication *self)
{
  g_settings_get
  (gsettings,
   key, "i",
   &(self->framelimit));
}

static void
on_fullscreen_changed(GSettings     *gsettings,
                      const gchar   *key,
                      DsApplication *self)
{
  gboolean flag;
  g_settings_get
  (gsettings,
   key, "b",
   &flag);

  Uint32 flags =
  SDL_GetWindowFlags(self->window);

  if(g_strcmp0(key, "fullscreen"))
  {
    g_settings_set
    (gsettings,
     "borderless",
     "b", FALSE);

    Uint32 mask = SDL_WINDOW_FULLSCREEN
                  | SDL_WINDOW_BORDERLESS;
    flags &= ~mask;

    if(flag == TRUE)
      flags |= SDL_WINDOW_FULLSCREEN;
  } else
  if(g_strcmp0(key, "borderless"))
  {
    g_settings_set
    (gsettings,
     "fullscreen",
     "b", FALSE);

    Uint32 mask = SDL_WINDOW_FULLSCREEN
                  | SDL_WINDOW_BORDERLESS;
    flags &= ~mask;

    if(flag == TRUE)
      flags |= SDL_WINDOW_BORDERLESS;
  }

  SDL_SetWindowFullscreen(self->window, flags);
}

gboolean
_ds_renderer_init(DsApplication  *self,
                  GCancellable   *cancellable,
                  GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);

  g_signal_connect
  (self->gsettings,
   "changed::width",
   G_CALLBACK(on_width_changed),
   self);

  g_signal_connect
  (self->gsettings,
   "changed::height",
   G_CALLBACK(on_height_changed),
   self);

  g_signal_connect
  (self->gsettings,
   "changed::framelimit",
   G_CALLBACK(on_framelimit_changed),
   self);

  g_signal_connect
  (self->gsettings,
   "changed::fullscreen",
   G_CALLBACK(on_fullscreen_changed),
   self);

  g_signal_connect
  (self->gsettings,
   "changed::borderless",
   G_CALLBACK(on_fullscreen_changed),
   self);

_error_:
return success;
}
