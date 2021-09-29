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
#include <ds_application.h>
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_renderer.h>
#include <SDL.h>

G_GNUC_INTERNAL
gboolean
_ds_renderer_step(DsRendererData* data)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
 * Frame timing
 *
 */

  static
  GLfloat deltaTime = 0.f;
  static
  GLfloat frameTime = 0.f;

  deltaTime = SDL_GetTicks() - frameTime;
  frameTime = SDL_GetTicks();

/*
 * Execute rendering pipeline
 *
 */

  DsApplication* self =
  (DsApplication*)
  g_application_get_default();
  ds_pipeline_execute(self->pipeline);

/*
 * Present display
 *
 */

  SDL_GL_SwapWindow(SDL_GL_GetCurrentWindow());
return G_SOURCE_CONTINUE;
}

/*
 * init / fini
 *
 */

static void
on_gl_debug_message(GLenum          source,
                    GLenum          type,
                    GLuint          id,
                    GLenum          severity,
                    GLsizei         length,
                    const GLchar   *message,
                    DsApplication  *self)
{
/*
 * Debug level
 *
 */

  GLogLevelFlags log_level;
  switch(severity)
  {
  case GL_DEBUG_SEVERITY_LOW:
    log_level = G_LOG_LEVEL_WARNING;
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    log_level = G_LOG_LEVEL_WARNING;
    break;
  case GL_DEBUG_SEVERITY_HIGH:
    log_level = G_LOG_LEVEL_CRITICAL;
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    log_level = G_LOG_LEVEL_MESSAGE;
    break;
  default:
    g_critical
    ("Unknown severity type '%i'\r\n",
     (gint) severity);
    g_assert_not_reached();
    break;
  }

/*
 * Enums values
 *
 */

  GType source_t = ds_gl_debug_source_get_type();
  GType type_t = ds_gl_debug_message_type_get_type();

  GEnumClass* source_c = g_type_class_peek(source_t);
  if G_UNLIKELY(source_c == NULL)
  {
    source_c =
    g_type_class_ref(source_t);

    g_object_set_data_full
    (G_OBJECT(self),
     "ds-application-gl-debug-source-type-class",
     source_c,
     (GDestroyNotify)
     g_type_class_unref);
  }

  GEnumClass* type_c = g_type_class_peek(type_t);
  if G_UNLIKELY(type_c == NULL)
  {
    type_c =
    g_type_class_ref(type_t);

    g_object_set_data_full
    (G_OBJECT(self),
     "ds-application-gl-debug-message-type-class",
     type_c,
     (GDestroyNotify)
     g_type_class_unref);
  }

  GEnumValue* source_v = g_enum_get_value(source_c, (gint) source);
  if G_UNLIKELY(source_v == NULL)
  {
    g_critical
    ("Unknown debug source type '%i'\r\n",
     (gint) source);
    g_assert_not_reached();
  }

  GEnumValue* type_v = g_enum_get_value(type_c, (gint) type);
  if G_UNLIKELY(type_v == NULL)
  {
    g_critical
    ("Unknown debug message type '%i'\r\n",
     (gint) type);
    g_assert_not_reached();
  }

/*
 * Log message
 *
 */

  g_log
  ("GL",
   log_level,
   "(%s: %i) %s: %s: %i: %.*s\r\n",
   G_STRFUNC,
   __LINE__,
   source_v->value_nick,
   type_v->value_nick,
   id,
   length,
   message);
}

static void
on_fullscreen_changed(GSettings     *gsettings,
                      const gchar   *key,
                      DsApplication *self)
{
  gboolean flag;
  g_settings_get(gsettings, key, "b", &flag);

  SDL_Window* window =
  SDL_GL_GetCurrentWindow();

  Uint32 flags =
  SDL_GetWindowFlags(window);

  if(g_strcmp0(key, "fullscreen") == 0)
  {
    g_settings_set(gsettings, "borderless", "b", FALSE);
    flags &= ~(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS);
    flags |= (flag) ? SDL_WINDOW_FULLSCREEN : 0;
  } else
  if(g_strcmp0(key, "borderless") == 0)
  {
    g_settings_set(gsettings, "fullscreen", "b", FALSE);
    flags &= ~(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS);
    flags |= (flag) ? SDL_WINDOW_BORDERLESS : 0;
  } else
  {
    g_assert_not_reached();
  }

  SDL_SetWindowFullscreen(window, flags);
}

G_GNUC_INTERNAL
gboolean
_ds_renderer_init(DsApplication  *self,
                  GSettings      *gsettings,
                  GCancellable   *cancellable,
                  GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * OpenGL initialization
 *
 */

  /* activate things */
  __gl_try_catch(
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  __gl_try_catch(
    glClearColor(0.f, 0.f, 0.f, 0.f);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  /* activate all debug messages */
  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
    __gl_try_catch(
      glEnable(GL_DEBUG_OUTPUT);
      glDebugMessageControl(_TRIPLET(GL_DONT_CARE), 0, NULL, TRUE);
      glDebugMessageCallback((GLDEBUGPROC) on_gl_debug_message, self);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
  }

/*
 * Connect signals
 * to settings holding
 * rendering parameters
 *
 */

  g_signal_connect
  (gsettings,
   "changed::fullscreen",
   G_CALLBACK(on_fullscreen_changed),
   self);

  g_signal_connect
  (gsettings,
   "changed::borderless",
   G_CALLBACK(on_fullscreen_changed),
   self);

_error_:
return success;
}
