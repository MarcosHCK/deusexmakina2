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
#include <ds_looper.h>
#include <ds_macros.h>
#include <ds_pipeline.h>
#include <ds_renderer.h>
#include <SDL.h>

static void
ds_renderer_g_initable_iface_init(GInitableIface* iface);

enum {
  conn_width,
  conn_height,
  conn_framelimit,
  conn_sensitivity,
  conn_fov,
  conn_fullscreen,
  conn_borderless,
  conn_number,
};

#define d self

static void update_view(DsRenderer* self, gfloat xrel, gfloat yrel);
static void update_projection(DsRenderer* self);

/*
 * Object definition
 *
 */

struct _DsRenderer
{
  DsLooper parent_instance;

  /*<private>*/
  GSettings* gsettings;
  DsPipeline* pipeline;
  SDL_Window* window;
  gulong connections[conn_number];

  /*<private>*/
  gfloat fov;
  gboolean framelimit;
  gint width;
  gint height;
  gint viewport_w;
  gint viewport_h;
  vec3 position;
  vec3 worldup;
  vec3 front_;
  gfloat yaw;
  gfloat pitch;
  gfloat sensitivity;

  /*<private>*/
  gfloat deltaTime;
  gfloat frameTime;
};

struct _DsRendererClass
{
  DsLooperClass parent_class;
};

enum {
  sig_update_view,
  sig_update_projection,
  sig_number,
};

static
guint signals[sig_number] = {0};

enum {
  prop_0,
  prop_gsettings,
  prop_pipeline,
  prop_window,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsRenderer,
 ds_renderer,
 DS_TYPE_LOOPER,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_renderer_g_initable_iface_init));

static void
on_width_changed(GSettings       *gsettings,
                 const gchar     *key,
                 DsRenderer      *self)
{
  g_settings_get(gsettings, key, "i", &(self->width));

  SDL_SetWindowSize(d->window, d->width, self->height);
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));
  update_projection(self);
}

static void
on_height_changed(GSettings      *gsettings,
                  const gchar    *key,
                  DsRenderer     *self)
{
  g_settings_get(gsettings, key, "i", &(d->height));

  SDL_SetWindowSize(d->window, d->width, d->height);
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));
  update_projection(self);
}

static void
on_framelimit_changed(GSettings      *gsettings,
                      const gchar    *key,
                      DsRenderer     *self)
{
  g_settings_get(gsettings, key, "b", &(d->framelimit));
}

static void
on_sensitivity_changed(GSettings       *gsettings,
                       const gchar     *key,
                       DsRenderer      *self)
{
  gdouble sensitivity;
  g_settings_get(gsettings, key, "d", &sensitivity);
  d->sensitivity = (gfloat) sensitivity;
}

static void
on_fov_changed(GSettings       *gsettings,
               const gchar     *key,
               DsRenderer      *self)
{
  gdouble fov;
  g_settings_get(gsettings, key, "d", &fov);
  d->fov = (gfloat) fov;

  update_projection(self);
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

static gboolean
ds_renderer_g_initable_iface_init_sync(GInitable* pself, GCancellable* cancellable, GError** error)
{
  DsRenderer* self = DS_RENDERER(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  gdouble fov, sensitivity;

/*
 * Initialize values
 *
 */

  glm_vec3_fill(d->position, 0.f);
  glm_vec3_fill(d->worldup, 0.f);
  glm_vec3_fill(d->front_, 0.f);

  d->worldup[1] = 1.f;
  d->front_[2] = -1.f;
  d->yaw = -90.f;
  d->pitch = 0.f;

/*
 * Get configuration values
 *
 */

  g_settings_get(d->gsettings, "fov", "d", &fov);
  g_settings_get(d->gsettings, "sensitivity", "d", &sensitivity);
  g_settings_get(d->gsettings, "framelimit", "b", &(self->framelimit));

  d->fov = (gfloat) fov;
  d->sensitivity = (gfloat) sensitivity;

/*
 * Get window metrics
 *
 */

  SDL_GetWindowSize(d->window, &(d->width), &(d->height));
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));

/*
 * Subscribe to configuration changes
 *
 */

  d->connections[conn_width] =
  g_signal_connect
  (d->gsettings,
   "changed::width",
   G_CALLBACK(on_width_changed),
   self);

  d->connections[conn_height] =
  g_signal_connect
  (d->gsettings,
   "changed::height",
   G_CALLBACK(on_height_changed),
   self);

  d->connections[conn_framelimit] =
  g_signal_connect
  (d->gsettings,
   "changed::framelimit",
   G_CALLBACK(on_framelimit_changed),
   self);

  d->connections[conn_sensitivity] =
  g_signal_connect
  (d->gsettings,
   "changed::sensitivity",
   G_CALLBACK(on_sensitivity_changed),
   self);

  d->connections[conn_fov] =
  g_signal_connect
  (d->gsettings,
   "changed::fov",
   G_CALLBACK(on_fov_changed),
   self);

  d->connections[conn_fullscreen] =
  g_signal_connect
  (d->gsettings,
   "changed::fullscreen",
   G_CALLBACK(on_fullscreen_changed),
   self);

  d->connections[conn_borderless] =
  g_signal_connect
  (d->gsettings,
   "changed::borderless",
   G_CALLBACK(on_fullscreen_changed),
   self);

/*
 * Perform some tweaks on GL
 *
 */

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
    glClearDepth(1.d);
  ,
    g_propagate_error(error, glerror);
    goto_error();
  );

  /* activate all debug messages */
#if !DEBUG
  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
#endif // DEBUG
    __gl_try_catch(
      glEnable(GL_DEBUG_OUTPUT);
      glDebugMessageControl(_TRIPLET(GL_DONT_CARE), 0, NULL, TRUE);
      glDebugMessageCallback((GLDEBUGPROC) on_gl_debug_message, self);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );
#if !DEBUG
  }
#endif // DEBUG

/*
 * Finish by updating things
 *
 */

  ds_renderer_force_update(self);
_error_:
return success;
}

static void
ds_renderer_g_initable_iface_init(GInitableIface* iface)
{
  iface->init = ds_renderer_g_initable_iface_init_sync;
}

static gboolean
ds_renderer_class_loop_step(DsLooper* pself)
{
  DsRenderer* self =  ((DsRenderer*) pself);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
 * Frame timing
 *
 */

  self->deltaTime = SDL_GetTicks()- self->frameTime;
  self->frameTime = SDL_GetTicks();

/*
 * Execute rendering pipeline
 *
 */

  ds_pipeline_execute(self->pipeline);

/*
 * Present display
 *
 */

  SDL_GL_SwapWindow(self->window);
return G_SOURCE_CONTINUE;
}

static void
ds_renderer_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsRenderer* self = DS_RENDERER(pself);
  switch(prop_id)
  {
  case prop_gsettings:
    g_set_object(&(self->gsettings), g_value_get_object(value));
    break;
  case prop_pipeline:
    g_set_object(&(self->pipeline), g_value_get_object(value));
    break;
  case prop_window:
    self->window = g_value_get_pointer(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_renderer_class_dispose(GObject* pself)
{
  DsRenderer* self = DS_RENDERER(pself);
  guint i;

  for(i = 0;
      i < conn_number;
      i++)
  if(d->connections[i] != 0)
  {
    g_signal_handler_disconnect(d->gsettings, d->connections[i]);
    d->connections[i] = 0;
  }

  g_clear_object(&(d->gsettings));
  g_clear_object(&(d->pipeline));
G_OBJECT_CLASS(ds_renderer_parent_class)->dispose(pself);
}

static void
ds_renderer_class_init(DsRendererClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);
  DsLooperClass* lclass = DS_LOOPER_CLASS(klass);

  lclass->loop_step = ds_renderer_class_loop_step;

  oclass->set_property = ds_renderer_class_set_property;
  oclass->dispose = ds_renderer_class_dispose;

  signals[sig_update_view] =
    g_signal_new
    ("update-view",
     G_TYPE_FROM_CLASS(klass),
     G_SIGNAL_ACTION,
     0,
     NULL,
     NULL,
     g_cclosure_marshal_VOID__POINTER,
     G_TYPE_NONE,
     1,
     G_TYPE_POINTER,
     G_TYPE_NONE);

  signals[sig_update_projection] =
    g_signal_new
    ("update-projection",
     G_TYPE_FROM_CLASS(klass),
     G_SIGNAL_ACTION,
     0,
     NULL,
     NULL,
     g_cclosure_marshal_VOID__POINTER,
     G_TYPE_NONE,
     1,
     G_TYPE_POINTER,
     G_TYPE_NONE);

  properties[prop_gsettings] =
    g_param_spec_object
    (_TRIPLET("gsettings"),
     G_TYPE_SETTINGS,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_pipeline] =
    g_param_spec_object
    (_TRIPLET("pipeline"),
     DS_TYPE_PIPELINE,
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  properties[prop_window] =
    g_param_spec_pointer
    (_TRIPLET("window"),
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static void
ds_renderer_init(DsRenderer* self)
{
}

/*
 * Object methods
 *
 */

#define position    self->position
#define worldup     self->worldup
#define front       self->front_
#define yaw         self->yaw
#define pitch       self->pitch
#define sensitivity self->sensitivity

static void
update_view(DsRenderer     *self,
            gfloat          xrel,
            gfloat          yrel)
{
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

/*
 * Emit signal
 *
 */

  GValue values[2] = {0};
  g_value_init(&(values[0]), G_TYPE_OBJECT);
  g_value_set_object(&(values[0]), self);
  g_value_init(&(values[1]), G_TYPE_POINTER);
  g_value_set_pointer(&(values[1]), camera);

  g_signal_emitv
  (values,
   signals[sig_update_view],
   0,
   NULL);

  g_value_unset(&(values[1]));
  g_value_unset(&(values[0]));
}

static void
update_projection(DsRenderer* self)
{
  mat4 projection;
  glm_perspective
  (glm_rad(d->fov),
     ((gfloat) d->width)
   / ((gfloat) d->height),
   0.1f,
   100.0f,
   projection);

  __gl_try_catch(
    glViewport(0, 0, d->viewport_w, d->viewport_h);
  ,
    g_critical
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(glerror->domain),
     glerror->code,
     glerror->message);
    g_error_free(glerror);
    g_assert_not_reached();
  );

/*
 * Emit signal
 *
 */

  GValue values[2] = {0};
  g_value_init(&(values[0]), G_TYPE_OBJECT);
  g_value_set_object(&(values[0]), self);
  g_value_init(&(values[1]), G_TYPE_POINTER);
  g_value_set_pointer(&(values[1]), projection);

  g_signal_emitv
  (values,
   signals[sig_update_projection],
   0,
   NULL);

  g_value_unset(&(values[1]));
  g_value_unset(&(values[0]));
}

#undef sensitivity
#undef pitch
#undef yaw
#undef front
#undef worldup
#undef position
#undef d

/**
 * ds_renderer_new: (constructor) (skip)
 * @gsettings: a #GSettings object.
 * @pipeline: (not nullable) (type Ds.Pipeline): pipeline object to execute.
 * @window: (not nullable): SDL window object.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Creates a new #DsRenderer instance, which uses @window
 * as rendering target and @pipeline as rendering pipeline.
 * @gsettings is used to get and watch rendering settings.
 *
 * Returns: (transfer full): a new #DsRenderer object.
 */
DsRenderer*
ds_renderer_new(GSettings      *gsettings,
                gpointer        pipeline,
                gpointer        window,
                GCancellable   *cancellable,
                GError        **error)
{
  return (DsRenderer*)
  g_initable_new
  (DS_TYPE_RENDERER,
   cancellable,
   error,
   "gsettings", gsettings,
   "pipeline", pipeline,
   "window", window,
   NULL);
}

/**
 * ds_renderer_force_update: (skip)
 * @renderer: a #DsRenderer object.
 *
 * Forces @renderer update, which is usually done on demand
 * after any change on rendering settings.
 *
 */
void
ds_renderer_force_update(DsRenderer* renderer)
{
  g_return_if_fail(DS_IS_RENDERER(renderer));

  update_projection(renderer);
  update_view(renderer, 0, 0);
}

/**
 * ds_renderer_look:
 * @renderer: a #DsRenderer object.
 * @xrel: relative displacement on X-axis.
 * @yrel: relative displacement on Y-axis.
 *
 * Performs some kind of "look around" action,
 * which ofcourse depends on game mechanics.
 *
 */
void
ds_renderer_look(DsRenderer  *renderer,
                 gfloat       xrel,
                 gfloat       yrel)
{
  g_return_if_fail(DS_IS_RENDERER(renderer));
  update_view(renderer, xrel, yrel);
}

/**
 * ds_renderer_move:
 * @renderer: a #DsRenderer object.
 * @xrel: relative displacement on X-axis.
 * @yrel: relative displacement on Y-axis.
 * @zrel: relative displacement on Z-axis.
 *
 * Moves viewpoint position.
 *
 */
void
ds_renderer_move(DsRenderer  *renderer,
                 gfloat       xrel,
                 gfloat       yrel,
                 gfloat       zrel)
{
  g_return_if_fail(DS_IS_RENDERER(renderer));

  vec3 vec_ = {xrel, yrel, zrel};
  glm_vec3_add(renderer->position, vec_, renderer->position);
  update_view(renderer, 0, 0);
}
