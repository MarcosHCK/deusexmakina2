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
#include <ds_luaobj.h>
#include <ds_macros.h>
#include <ds_mvpholder.h>
#include <ds_settings.h>
#include <GL/glew.h>
#include <SDL.h>
#undef main

G_DEFINE_QUARK(ds-application-error-quark,
               ds_application_error);

static
void ds_application_g_initiable_iface_init(GInitableIface* iface);

/*
 * Clean-up
 *
 */

static void
_glew_fini0(guint var)
{
}

static void
_SDL_GL_DeleteContext0(SDL_GLContext var)
{
  (var == NULL) ? NULL : (var = (SDL_GL_DeleteContext (var), NULL));
}

static void
_SDL_DestroyWindow0(SDL_Window* var)
{
  (var == NULL) ? NULL : (var = (SDL_DestroyWindow (var), NULL));
}

static void
_SDL_fini0(guint var)
{
  (var == 0) ? 0 : (var = (SDL_Quit (), 0));
}

static void
_lua_close0(lua_State* var)
{
  (var == NULL) ? NULL : (var = (_ds_lua_fini(var), lua_close (var), NULL));
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
  guint sdl_init;
  SDL_Window* window;
  SDL_GLContext* glctx;
  guint glew_init;
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

#define L         (self->L)
#define sdl_init  (self->priv->sdl_init)
#define glew_init (self->priv->glew_init)
#define pipeline  (self->pipeline)

static gboolean
ds_application_g_initiable_iface_init_sync(GInitable     *pself,
                                           GCancellable  *cancellable,
                                           GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsApplication* self =
  DS_APPLICATION(pself);
  gboolean debug = FALSE;
  gint return_ = 0;

  DsSettings* dssettings = NULL;
  GSettings* gsettings = NULL;
  SDL_Window* window = NULL;
  SDL_GLContext* glctx = NULL;
  DsRenderer* renderer = NULL;
  DsEvents* events = NULL;

/*
 * Debug flag
 *
 */

  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
    debug = TRUE;
  }

/*
 * Settings
 *
 */

  if G_UNLIKELY(debug == TRUE)
    dssettings =
    ds_settings_new(ABSTOPBUILDDIR "/settings/", cancellable, &tmp_err);
  else
    dssettings =
    ds_settings_new(SCHEMASDIR, cancellable, &tmp_err);

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

  L = luaL_newstate();
  if G_UNLIKELY(L == NULL)
  {
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_LUA_INIT,
     "luaL_newstate(): failed!: unknown error\r\n");
    goto_error();
  }

  success =
  _ds_lua_init(L, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  if G_UNLIKELY(debug == TRUE)
  {
    return_ =
    luaL_loadfile(L, ABSTOPBUILDDIR "/scripts/patches.lua");
    lua_pushstring(L, ABSTOPBUILDDIR "/scripts/");
  }
  else
  {
    return_ =
    luaL_loadfile(L, PKGLIBEXECDIR "/patches.lua");
    lua_pushstring(L, PKGLIBEXECDIR);
  }

  if G_UNLIKELY
    (lua_isfunction(L, -2) == FALSE)
  {
    const gchar* err =
    lua_tostring(L, -2);

    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_FAILED,
     "luaL_loadfile(): failed!: %s\r\n",
     (err != NULL) ? err : "unknown error");
    goto_error();
  }

  success =
  luaD_xpcall(L, 1, 0, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  events =
  ds_events_new(L, cancellable, &tmp_err);
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
 * SDL2
 *
 */

  return_ =
  SDL_Init
  (SDL_INIT_VIDEO
   | SDL_INIT_AUDIO
   | SDL_INIT_EVENTS);

  if G_UNLIKELY(0 > return_)
  {
    const gchar* err =
    SDL_GetError();

    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_SDL2_INIT,
     "SDL_Init(): failed!: %i: %s\r\n",
     return_, err ? err : "(null)");
    goto_error();
  }
  else
  {
    sdl_init = 1;
  }

/*
 * Main window
 *
 */

  Uint32 flags = SDL_WINDOW_OPENGL;
  gboolean  fullscreen = FALSE,
            borderless = FALSE;

  gint  width = 0,
        height = 0;

  g_settings_get(gsettings, "fullscreen", "b", &fullscreen);
  g_settings_get(gsettings, "borderless", "b", &borderless);
  g_settings_get(gsettings, "width", "i", &width);
  g_settings_get(gsettings, "height", "i", &height);

  flags |= fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
  flags |= borderless ? SDL_WINDOW_BORDERLESS : 0;

  if G_UNLIKELY(debug == TRUE)
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_DEBUG_FLAG, TRUE);
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);

  window =
  SDL_CreateWindow
  (GAPPNAME,
   SDL_WINDOWPOS_UNDEFINED,
   SDL_WINDOWPOS_UNDEFINED,
   width,
   height,
   flags);

  if G_UNLIKELY(window == NULL)
  {
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_SDL2_WINDOW_INIT,
     "SDL_CreateWindow(): failed!: %s\r\n",
     SDL_GetError());
    goto_error();
  }

  glctx =
  SDL_GL_CreateContext(window);
  if G_UNLIKELY(glctx == NULL)
  {
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_GL_CONTEXT_INIT,
     "SDL_GL_CreateContext(): failed!: %s\r\n",
     SDL_GetError());
    _SDL_DestroyWindow0(window);
    goto_error();
  }

  self->priv->window = window;
  self->priv->glctx = glctx;
  SDL_ShowCursor(1);

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
 * Execute setup script
 *
 */

  if G_UNLIKELY(debug == TRUE)
    return_ =
    luaL_loadfile(L, ABSTOPBUILDDIR "/scripts/setup.lua");
  else
    return_ =
    luaL_loadfile(L, PKGLIBEXECDIR "/setup.lua");

  if G_UNLIKELY
    (lua_isfunction(L, -1) == FALSE)
  {
    const gchar* err =
    lua_tostring(L, -1);

    g_set_error
    (error,
     DS_LUA_ERROR,
     DS_LUA_ERROR_FAILED,
     "luaL_loadfile(): failed!: %s\r\n",
     (err != NULL) ? err : "unknown error");
    goto_error();
  }

  luaD_pushobject(L, (GObject*) self);
  luaD_pushobject(L, (GObject*) cancellable);

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
return success;
}

#undef L
#undef sdl_init
#undef glew_init
#undef pipeline

static
void ds_application_g_initiable_iface_init(GInitableIface* iface) {
  iface->init = ds_application_g_initiable_iface_init_sync;
}

static
void ds_application_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec)
{
  DsApplication* self = DS_APPLICATION(pself);
  switch(prop_id)
  {
  case prop_lua_state:
    g_value_set_pointer(value, self->L);
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
  _SDL_GL_DeleteContext0(self->priv->glctx);
  _SDL_DestroyWindow0(self->priv->window);
  _SDL_fini0(self->priv->sdl_init);

  /* Find out if Lua uses application object it won't be destroyed  */
  /* because Lua holds a reference                                  */
  _lua_close0(self->L);
G_OBJECT_CLASS(ds_application_parent_class)->finalize(pself);
}

static
void ds_application_class_dispose(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);
  g_clear_object(&(self->events));
  g_clear_object(&(self->renderer));
  g_clear_object(&(self->pipeline));
  g_clear_object(&(self->priv->cache_provider));
  g_clear_object(&(self->priv->data_provider));
  g_clear_object(&(self->priv->gsettings));
  g_clear_object(&(self->priv->dssettings));
G_OBJECT_CLASS(ds_application_parent_class)->dispose(pself);
}

static
void ds_application_class_init(DsApplicationClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

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

/*
 * on_activate()
 *
 */

static
void on_activate(DsApplication* self) {
  GMainContext* context =
  g_main_context_default();
  GSource* source = NULL;

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
  g_application_hold
  (G_APPLICATION(self));
}

/*
 * main()
 *
 */

int
main(int    argc,
     char  *argv[])
{
/*
 * Create application
 *
 */

  GError* tmp_err = NULL;
  GApplication* app = (GApplication*)

  g_initable_new
  (DS_TYPE_APPLICATION,
   NULL,
   &tmp_err,
   "application-id", GAPPNAME,
   "flags", G_APPLICATION_FLAGS_NONE,
   NULL);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_critical
    ("%s: %i: %s\r\n",
     g_quark_to_string(tmp_err->domain),
     tmp_err->code,
     tmp_err->message);
    g_error_free(tmp_err);
    g_assert_not_reached();
  }

/*
 * Subscribe to 'activate'
 * signal
 *
 */

  g_signal_connect
  (app,
   "activate",
   G_CALLBACK(on_activate),
   NULL);

/*
 * Run main loop
 *
 */

  int status =
  g_application_run(app, argc, argv);

/*
 * Finalize application object
 *
 */

  lua_gc(DS_APPLICATION(app)->L, LUA_GCCOLLECT, 1);         /* Collects all unused references, thus it may destroy some objects */
  g_clear_pointer(&(DS_APPLICATION(app)->L), _lua_close0);  /* Release a possibly held reference to application object          */
  g_assert_finalize_object(app);                            /* Finally destroy application object                               */
return status;
}
