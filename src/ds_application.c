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
#include <ds_folder_provider.h>
#include <ds_looper.h>
#include <ds_macros.h>
#include <ds_mvpholder.h>
#include <ds_pencil.h>
#include <ds_settings.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <luad_core.h>
#undef main

/**
 * SECTION:dsapplication
 * @Short_description: Main application
 * @Title: DsApplication
 *
 * DsApplication is a central part on deusexmakina2, since
 * it manages startup and shutdown procedures, internal
 * state management and in general all things going on.
 */

G_DEFINE_QUARK(ds-application-error-quark,
               ds_application_error);

static void
ds_application_g_initiable_iface_init(GInitableIface* iface);

/*
 * Clean-up
 *
 */

static void
_glew_fini0(guint var)
{
}

static void
_glfw_DestroyWindow0(GLFWwindow* var)
{
  GData** datalist = (var) ? glfwGetWindowUserPointer(var) : NULL;
  g_datalist_clear(datalist);

  (var == NULL) ? NULL : (var = (glfwDestroyWindow (var), NULL));
}

static void
_glfw_fini0(guint var)
{
  (var == 0) ? 0 : (var = (glfwTerminate (), 0));
}

/*
 * Object definition
 *
 */

struct _DsApplicationPrivate
{
  DsSettings* dssettings;
  GSettings* gsettings;
  DsDataProvider* data_provider;
  DsCacheProvider* cache_provider;
  guint glfw_init;
  GLFWwindow* window;
  GData* window_datalist;
  guint glew_init;
  DsPencil* pencil;
};

enum {
  prop_0,

/*
 * Initialization-time variables,
 * therefore they are READ-ONLY.
 *
 */

  prop_lua_state,
  prop_pipeline,
  prop_renderer,
  prop_events,

/*
 * Property number, a convenience way
 * to automatically get how many properties
 * this object has.
 *
 */
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsApplication,
 ds_application,
 G_TYPE_APPLICATION,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_application_g_initiable_iface_init)
 G_ADD_PRIVATE(DsApplication));

#define L         (self->lua)
#define glfw_init (self->priv->glfw_init)
#define glew_init (self->priv->glew_init)

static gboolean
ds_application_g_initiable_iface_init_sync(GInitable     *pself,
                                           GCancellable  *cancellable,
                                           GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsApplication* self = DS_APPLICATION(pself);
  DsApplicationPrivate* priv = self->priv;
  gint return_ = 0;

  DsSettings* dssettings = NULL;
  GSettings* gsettings = NULL;
  GLFWwindow* window = NULL;
  DsPencil* pencil = NULL;
  DsPipeline* pipeline = NULL;
  DsRenderer* renderer = NULL;
  DsEvents* events = NULL;

  GFile* current = NULL;
  GFile* child = NULL;

  /* keep this sync with luad_lib.c */
  current = g_file_new_for_path(".");

/*
 * Settings
 *
 */


#if DEVELOPER
  g_set_object(&child, g_file_get_child(current, "settings/"));
  dssettings =
  ds_settings_new(g_file_peek_path(child), cancellable, &tmp_err);
#else
  dssettings =
  ds_settings_new(SCHEMASDIR, cancellable, &tmp_err);
#endif // DEVELOPER
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    _g_object_unref0(dssettings);
    goto_error();
  }

  const gchar* schema_id = GAPPNAME;

  gsettings =
  ds_settings_get_settings(dssettings, schema_id);
  if G_UNLIKELY(gsettings == NULL)
  {
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_GSETTINGS_INIT,
     "ds_settings_get_settings(): failed!: schema '%s' not found\r\n",
     schema_id);
    _g_object_unref0(dssettings);
    _g_object_unref0(gsettings);
    goto_error();
  }

  self->priv->dssettings = dssettings;
  self->priv->gsettings = gsettings;

/*
 * Saves manager
 *
 */

  self->priv->data_provider =
  ds_data_provider_new(cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  self->priv->cache_provider =
  ds_cache_provider_new(cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Lua
 *
 */

  L = luaD_new(&tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

#if DEVELOPER
  g_set_object(&child, g_file_get_child(current, "scripts/patches.lua"));
  luaL_loadfile(L, g_file_peek_path(child));
#else
  luaL_loadfile(L, PKGLIBEXECDIR "/patches.lua");
#endif // DEVELOPER

  if G_UNLIKELY
    (lua_isfunction(L, -1) == FALSE)
  {
    const gchar* err =
    lua_tostring(L, -1);

    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_FAILED,
     "luaL_loadfile(): failed!: %s\r\n",
     (err != NULL) ? err : "unknown error");
    goto_error();
  }

#if DEVELOPER
  g_set_object(&child, g_file_get_child(current, "scripts/"));
  lua_pushstring(L, g_file_peek_path(child));
  g_set_object(&child, g_file_get_child(current, "gir/"));
  lua_pushstring(L, g_file_peek_path(child));
#else
  lua_pushstring(L, PKGLIBEXECDIR);
  lua_pushstring(L, GIRDIR);
#endif // DEVELOPER

  success =
  luaD_xpcall(L, 2, 0, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * GLFW
 *
 */

  return_ = glfwInit();
  if G_UNLIKELY(return_ != GLFW_TRUE)
  {
    const gchar* err = NULL;
    glfwGetError(&err);

    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_GLFW_INIT,
     "glfwInit(): failed!: %i: %s\r\n",
     return_, err ? err : "(null)");
    goto_error();
  }
  else
  {
    glfw_init = 1;
  }

/*
 * Main window
 *
 */

  gboolean  fullscreen = FALSE,
            borderless = FALSE;
  gint  width = 0,
        height = 0;
  GLFWmonitor* monitor = NULL;

  g_settings_get(gsettings, "fullscreen", "b", &fullscreen);
  g_settings_get(gsettings, "borderless", "b", &borderless);
  g_settings_get(gsettings, "width", "i", &width);
  g_settings_get(gsettings, "height", "i", &height);

  if(fullscreen == TRUE && borderless == TRUE)
  {
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    width = mode->width;
    height = mode->height;

    g_settings_set(gsettings, "width", "i", width);
    g_settings_set(gsettings, "height", "i", height);
  }
  else
  if(fullscreen == TRUE)
  {
    monitor = glfwGetPrimaryMonitor();
  }

  glfwWindowHint(GLFW_DECORATED, (borderless) ? GLFW_FALSE : GLFW_TRUE);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
#if DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // DEBUG

  window =
  glfwCreateWindow(width, height, GAPPNAME, monitor, NULL);
  if G_UNLIKELY(window == NULL)
  {
    const gchar* err = NULL;
    glfwGetError(&err);

    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_GLFW_WINDOW_INIT,
     "glfwCreateWindow(): failed!: %s\r\n",
     err ? err : "(null)");
    goto_error();
  }

  glfwMakeContextCurrent(window);
  self->priv->window = window;

  GData** datalist = &(self->priv->window_datalist);
  g_datalist_init(datalist);
  glfwSetWindowUserPointer(window, datalist);

/*
 * GLEW
 *
 */

  return_ = glewInit();
  if G_UNLIKELY(return_ != GLEW_OK)
  {
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_GLEW_INIT,
     "glewInit(): failed!: %i: %s\r\n",
     return_, (gchar*) glewGetErrorString(return_));
    goto_error();
  }
  else
  {
    glew_init = 1;
  }

/*
 * Pencil
 *
 */

  pencil =
  ds_pencil_new(&tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }
  else
  {
    priv->pencil = pencil;
  }

/*
 * Pipeline
 *
 */

  pipeline =
  ds_pipeline_new(cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }
  else
  {
    self->pipeline = pipeline;
  }

/*
 * Renderer
 *
 */

  renderer =
  ds_renderer_new(gsettings, pipeline, window, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    _g_object_unref0(renderer);
    goto_error();
  }
  else
  {
    self->renderer = renderer;
  }

  g_signal_connect_swapped
  (renderer,
   "update-projection",
   G_CALLBACK(ds_mvp_holder_set_projection),
   pipeline);

  g_signal_connect_swapped
  (renderer,
   "update-view",
   G_CALLBACK(ds_mvp_holder_set_view),
   pipeline);

  /* force matrix update */
  ds_renderer_force_update(renderer);

/*
 * Events
 *
 */

  events =
  ds_events_new(gsettings, window, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    _g_object_unref0(events);
    goto_error();
  }
  else
  {
    self->events = events;
  }

/*
 * Execute setup script
 *
 */

  return_ =
#if DEVELOPER
  g_set_object(&child, g_file_get_child(current, "scripts/setup.lua"));
  luaL_loadfile(L, g_file_peek_path(child));
#else
  luaL_loadfile(L, PKGLIBEXECDIR "/setup.lua");
#endif // DEVELOPER

  if G_UNLIKELY
    (lua_isfunction(L, -1) == FALSE)
  {
    const gchar* err =
    lua_tostring(L, -1);

    g_set_error
    (error,
     LUAD_ERROR,
     LUAD_ERROR_FAILED,
     "luaL_loadfile(): failed!: %s\r\n",
     (err != NULL) ? err : "unknown error");
    goto_error();
  }

  lua_pushlightuserdata(L, self);
  if G_UNLIKELY(cancellable != NULL)
    lua_pushlightuserdata(L, cancellable);
  else
    lua_pushnil(L);

  success =
  luaD_xpcall(L, 2, 0, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  lua_settop(L, 0);
  lua_gc(L, LUA_GCCOLLECT, 1);

_error_:
  _g_object_unref0(current);
  _g_object_unref0(child);
return success;
}

#undef L
#undef glfw_init
#undef glew_init

static
void ds_application_g_initiable_iface_init(GInitableIface* iface) {
  iface->init = ds_application_g_initiable_iface_init_sync;
}

static
void ds_application_class_activate(GApplication* pself)
{
  DsApplication* self = DS_APPLICATION(pself);
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * Initialize ourself
 *
 */

  success =
  g_initable_init(G_INITABLE(pself), g_cancellable_get_current(), &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("(%s: %i): %s: %i: %s\r\n",
     G_STRFUNC,
     __LINE__,
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    return;
  }

/*
 * Start loopers
 *
 */

  ds_looper_start(DS_LOOPER(self->renderer));
  ds_looper_start(DS_LOOPER(self->events));

/*
 * Increase hold count
 *
 */

  g_application_hold(pself);
}

static
void ds_application_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec)
{
  DsApplication* self = DS_APPLICATION(pself);
  switch(prop_id)
  {
  case prop_lua_state:
    g_value_set_pointer(value, self->lua);
    break;
  case prop_pipeline:
    g_value_set_object(value, self->pipeline);
    break;
  case prop_renderer:
    g_value_set_object(value, self->renderer);
    break;
  case prop_events:
    g_value_set_object(value, self->events);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_application_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsApplication* self = DS_APPLICATION(pself);
  switch(prop_id)
  {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_application_class_finalize(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);
  _glew_fini0(self->priv->glew_init);
  _glfw_DestroyWindow0(self->priv->window);
  _glfw_fini0(self->priv->glfw_init);
G_OBJECT_CLASS(ds_application_parent_class)->finalize(pself);
}

static
void ds_application_class_dispose(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);
  g_clear_object(&(self->events));
  g_clear_object(&(self->renderer));
  g_clear_object(&(self->pipeline));
  g_clear_object(&(self->priv->pencil));
  g_clear_object(&(self->priv->cache_provider));
  g_clear_object(&(self->priv->data_provider));
  g_clear_object(&(self->priv->gsettings));
  g_clear_object(&(self->priv->dssettings));
G_OBJECT_CLASS(ds_application_parent_class)->dispose(pself);
}

static
void ds_application_class_init(DsApplicationClass* klass) {
  GApplicationClass* aclass = G_APPLICATION_CLASS(klass);
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  aclass->activate = ds_application_class_activate;

  oclass->get_property = ds_application_class_get_property;
  oclass->set_property = ds_application_class_set_property;
  oclass->finalize = ds_application_class_finalize;
  oclass->dispose = ds_application_class_dispose;

  properties[prop_lua_state] =
    g_param_spec_pointer
    (_TRIPLET("lua-state"),
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_pipeline] =
    g_param_spec_object
    (_TRIPLET("pipeline"),
     DS_TYPE_PIPELINE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_renderer] =
    g_param_spec_object
    (_TRIPLET("renderer"),
     DS_TYPE_RENDERER,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_events] =
    g_param_spec_object
    (_TRIPLET("events"),
     DS_TYPE_EVENTS,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static
void ds_application_init(DsApplication* self) {
  self->priv = ds_application_get_instance_private(self);
}
