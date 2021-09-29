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
#include <ds_gl.h>
#include <ds_macros.h>
#include <ds_renderer_data.h>
#include <SDL.h>

static
void ds_renderer_data_g_initable_init(GInitableIface* iface);
static
void update_view(DsRendererData* self, gfloat x, gfloat y, gfloat rx, gfloat ry);
static
void update_projection(DsRendererData* self);

/*
 * Object definition
 *
 */

#define n_connections 5

struct _DsRendererDataPrivate
{
  GSettings* gsettings;
  SDL_Window* window;

  gulong connections[n_connections];

  gfloat fov;
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
};

#define d self->priv

enum {
  sig_update_view,
  sig_update_projection,
  sig_number,
};

static
guint signals[sig_number] = {0};

enum {
  prop_0,
  prop_settings,
  prop_window,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsRendererData,
 ds_renderer_data,
 G_TYPE_OBJECT,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_renderer_data_g_initable_init)
 G_ADD_PRIVATE(DsRendererData));

static void
on_width_changed(GSettings       *gsettings,
                 const gchar     *key,
                 DsRendererData  *self)
{
  g_settings_get(gsettings, key, "i", &(d->width));

  SDL_SetWindowSize(d->window, d->width, d->height);
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));
  update_projection(self);
}

static void
on_height_changed(GSettings      *gsettings,
                  const gchar    *key,
                  DsRendererData *self)
{
  g_settings_get(gsettings, key, "i", &(d->height));

  SDL_SetWindowSize(d->window, d->width, d->height);
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));
  update_projection(self);
}

static void
on_framelimit_changed(GSettings      *gsettings,
                      const gchar    *key,
                      DsRendererData *self)
{
  g_settings_get(gsettings, key, "b", &(self->framelimit));
}

static void
on_sensitivity_changed(GSettings       *gsettings,
                       const gchar     *key,
                       DsRendererData  *self)
{
  gdouble sensitivity;
  g_settings_get(gsettings, key, "d", &sensitivity);
  d->sensitivity = (gfloat) sensitivity;
}

static void
on_fov_changed(GSettings       *gsettings,
               const gchar     *key,
               DsRendererData  *self)
{
  gdouble fov;
  g_settings_get(gsettings, key, "d", &fov);
  d->fov = (gfloat) fov;

  update_projection(self);
}

static gboolean
ds_renderer_data_g_initable_init_sync(GInitable* pself, GCancellable* cancellable, GError** error)
{
  DsRendererData* self = DS_RENDERER_DATA(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

  glm_vec3_fill(d->position, 0.f);
  glm_vec3_fill(d->worldup, 0.f);
  glm_vec3_fill(d->front_, 0.f);

  d->worldup[1] = 1.f;
  d->front_[2] = -1.f;
  d->yaw = -90.f;
  d->pitch = 0.f;

  gdouble fov, sensitivity;
  g_settings_get(d->gsettings, "fov", "d", &fov);
  g_settings_get(d->gsettings, "sensitivity", "d", &sensitivity);
  g_settings_get(d->gsettings, "framelimit", "b", &(self->framelimit));

  SDL_GetWindowSize(d->window, &(d->width), &(d->height));
  SDL_GL_GetDrawableSize(d->window, &(d->viewport_w), &(d->viewport_h));

  d->fov = (gfloat) fov;
  d->sensitivity = (gfloat) sensitivity;

  d->connections[0] =
  g_signal_connect
  (d->gsettings,
   "changed::width",
   G_CALLBACK(on_width_changed),
   self);

  d->connections[1] =
  g_signal_connect
  (d->gsettings,
   "changed::height",
   G_CALLBACK(on_height_changed),
   self);

  d->connections[2] =
  g_signal_connect
  (d->gsettings,
   "changed::framelimit",
   G_CALLBACK(on_framelimit_changed),
   self);

  d->connections[3] =
  g_signal_connect
  (d->gsettings,
   "changed::sensitivity",
   G_CALLBACK(on_sensitivity_changed),
   self);

  d->connections[4] =
  g_signal_connect
  (d->gsettings,
   "changed::fov",
   G_CALLBACK(on_fov_changed),
   self);

  ds_renderer_data_force_update(self);
_error_:
return success;
}

static
void ds_renderer_data_g_initable_init(GInitableIface* iface) {
  iface->init = ds_renderer_data_g_initable_init_sync;
}

static
void ds_renderer_data_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsRendererData* self = DS_RENDERER_DATA(pself);
  switch(prop_id)
  {
  case prop_settings:
    g_set_object(&(self->priv->gsettings), g_value_get_object(value));
    break;
  case prop_window:
    self->priv->window = g_value_get_pointer(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_renderer_data_class_dispose(GObject* pself) {
  DsRendererData* self = DS_RENDERER_DATA(pself);
  guint i;

  for(i = 0;
      i < n_connections;
      i++)
  if(d->connections[i] != 0)
  {
    g_signal_handler_disconnect(d->gsettings, d->connections[i]);
    d->connections[i] = 0;
  }

  g_clear_object(&(d->gsettings));
G_OBJECT_CLASS(ds_renderer_data_parent_class)->dispose(pself);
}

static
void ds_renderer_data_class_init(DsRendererDataClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  oclass->set_property = ds_renderer_data_class_set_property;
  oclass->dispose = ds_renderer_data_class_dispose;

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

  properties[prop_settings] =
    g_param_spec_object
    (_TRIPLET("settings"),
     G_TYPE_SETTINGS,
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

static
void ds_renderer_data_init(DsRendererData* self)
{
  self->priv = ds_renderer_data_get_instance_private(self);
}

/*
 * Object methods
 *
 */

DsRendererData*
ds_renderer_data_new(GSettings     *gsettings,
                     gpointer       window,
                     GCancellable  *cancellable,
                     GError       **error)
{
  return (DsRendererData*)
  g_initable_new
  (DS_TYPE_RENDERER_DATA,
   cancellable,
   error,
   "settings", gsettings,
   "window", window,
   NULL);
}

#define position    d->position
#define worldup     d->worldup
#define front       d->front_
#define yaw         d->yaw
#define pitch       d->pitch
#define sensitivity d->sensitivity

static void
update_view(DsRendererData *self,
            gfloat          x,
            gfloat          y,
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
update_projection(DsRendererData* self)
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

void
ds_renderer_data_force_update(DsRendererData* data)
{
  g_return_if_fail(DS_IS_RENDERER_DATA(data));

  update_projection(data);
  update_view(data, 0, 0, 0, 0);
}

void
ds_renderer_data_look(DsRendererData *data,
                      gfloat          x,
                      gfloat          y,
                      gfloat          xrel,
                      gfloat          yrel)
{
  g_return_if_fail(DS_IS_RENDERER_DATA(data));
  update_view(data, x, y, xrel, yrel);
}
