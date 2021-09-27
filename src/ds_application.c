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
#include <ds_error.h>
#include <ds_events.h>
#include <ds_font.h>
#include <ds_luaobj.h>
#include <ds_model.h>
#include <ds_renderer.h>
#include <ds_skybox.h>
#include <ds_text.h>
#include <glib/gi18n.h>
#undef main

G_DEFINE_QUARK(ds-application-error-quark,
               ds_application_error);

static
void ds_application_g_initiable_iface_init(GInitableIface* iface);

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

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

enum {
  prop_0,
  prop_dssettings,
  prop_gsettings,
  prop_basedatadir,
  prop_basecachedir,
  prop_savesdir,
  prop_glcachedir,
  prop_lua_state,
  prop_sdl_window,
  prop_gl_context,
  prop_width,
  prop_height,
  prop_viewport_width,
  prop_viewport_height,
  prop_framelimit,
  prop_pipeline,
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
  ds_application_g_initiable_iface_init));

#define dssettings self->dssettings
#define gsettings self->gsettings
#define basedatadir self->basedatadir
#define basecachedir self->basecachedir
#define savesdir self->savesdir
#define glcachedir self->glcachedir
#define L self->L
#define sdl_init self->sdl_init
#define window self->window
#define glctx self->glctx
#define glew_init self->glew_init
#define pipeline self->pipeline

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

/*
 * Ensure class initialization
 *
 */

  g_type_ensure(DS_TYPE_APPLICATION);
  g_type_ensure(DS_TYPE_ERROR);
  g_type_ensure(DS_TYPE_FONT);
  g_type_ensure(DS_TYPE_MODEL);
  g_type_ensure(DS_TYPE_PIPELINE);
  g_type_ensure(DS_TYPE_SAVE);
  g_type_ensure(DS_TYPE_SETTINGS);
  g_type_ensure(DS_TYPE_SHADER);
  g_type_ensure(DS_TYPE_SKYBOX);
  g_type_ensure(DS_TYPE_TEXT);

  g_type_ensure(ds_gl_error_get_type());
  g_type_ensure(ds_gl_object_type_get_type());
  g_type_ensure(ds_gl_texture_type_get_type());
  g_type_ensure(ds_gl_debug_source_get_type());
  g_type_ensure(ds_gl_debug_message_type_get_type());
  g_type_ensure(ds_gl_debug_severity_get_type());

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
    goto_error();
  }

/*
 * Saves manager
 *
 */

  basedatadir =
  _ds_base_data_dir_pick(cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  basecachedir =
  _ds_base_cache_dir_pick(cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  savesdir =
  _ds_base_dirs_child("saves", basedatadir, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  glcachedir =
  _ds_base_dirs_child("glcache", basecachedir, cancellable, &tmp_err);
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

  success =
  _ds_events_init(L, gsettings, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
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

  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
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

  self->renderer_data.width = width;
  self->renderer_data.height = height;

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
    goto_error();
  }

  SDL_GL_GetDrawableSize
  (window,
   &(self->renderer_data.viewport_w),
   &(self->renderer_data.viewport_h));

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

  success =
  _ds_renderer_init(self, gsettings, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
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

  /* push object */
  luaD_pushobject(L, G_OBJECT(pipeline));
  lua_setglobal(L, "pipeline");

  /* setup mvp matrix components */
  _ds_renderer_data_set_projection(self, pipeline);
  _ds_renderer_data_set_view(self, pipeline, 0.f, 0.f, 0.f, 0.f);

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

  luaD_pushobject(L, (GObject*) cancellable);

  success =
  luaD_xpcall(L, 1, 0, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Update pipeline
 *
 */

  success =
  ds_pipeline_update(pipeline, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Get Lua stack ready
 *
 */

  lua_settop(L, 0);

_error_:
  if G_UNLIKELY(success == FALSE)
  {
    g_clear_object(&pipeline);
    g_clear_handle_id
    (&glew_init, _glew_fini0);
    g_clear_pointer
    (&glctx, _SDL_GL_DeleteContext0);
    g_clear_pointer
    (&window, _SDL_DestroyWindow0);
    g_clear_handle_id
    (&sdl_init, _SDL_fini0);
    g_clear_pointer
    (&L, _lua_close0);
    g_clear_object(&glcachedir);
    g_clear_object(&savesdir);
    g_clear_object(&basecachedir);
    g_clear_object(&basedatadir);
    g_clear_object(&gsettings);
    g_clear_object(&dssettings);
  }
return success;
}

#undef dssettings
#undef gsettings
#undef basedatadir
#undef basecachedir
#undef savesdir
#undef glcachedir
#undef L
#undef sdl_init
#undef window
#undef glctx
#undef glew_init
#undef pipeline

static
void ds_application_g_initiable_iface_init(GInitableIface* iface) {
  iface->init = ds_application_g_initiable_iface_init_sync;
}

static
void ds_application_class_finalize(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);
  _glew_fini0(self->glew_init);
  _SDL_GL_DeleteContext0(self->glctx);
  _SDL_DestroyWindow0(self->window);
  _SDL_fini0(self->sdl_init);
  _lua_close0(self->L);
G_OBJECT_CLASS(ds_application_parent_class)->finalize(pself);
}

static
void ds_application_class_dispose(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);
  g_clear_object(&(self->pipeline));
  g_clear_object(&(self->glcachedir));
  g_clear_object(&(self->savesdir));
  g_clear_object(&(self->basecachedir));
  g_clear_object(&(self->basedatadir));
  g_clear_object(&(self->gsettings));
  g_clear_object(&(self->dssettings));
G_OBJECT_CLASS(ds_application_parent_class)->dispose(pself);
}

static
void ds_application_class_get_property(GObject* pself, guint prop_id, GValue* value, GParamSpec* pspec) {
  DsApplication* self = DS_APPLICATION(pself);
  switch(prop_id)
  {
  case prop_dssettings:
    g_value_set_object(value, self->dssettings);
    break;
  case prop_gsettings:
    g_value_set_object(value, self->gsettings);
    break;
  case prop_basedatadir:
    g_value_set_object(value, self->basedatadir);
    break;
  case prop_basecachedir:
    g_value_set_object(value, self->basecachedir);
    break;
  case prop_savesdir:
    g_value_set_object(value, self->savesdir);
    break;
  case prop_glcachedir:
    g_value_set_object(value, self->glcachedir);
    break;
  case prop_lua_state:
    g_value_set_pointer(value, self->L);
    break;
  case prop_sdl_window:
    g_value_set_pointer(value, self->window);
    break;
  case prop_gl_context:
    g_value_set_pointer(value, self->glctx);
    break;
  case prop_width:
    g_value_set_int(value, self->renderer_data.width);
    break;
  case prop_height:
    g_value_set_int(value, self->renderer_data.height);
    break;
  case prop_viewport_width:
    g_value_set_int(value, self->renderer_data.viewport_w);
    break;
  case prop_viewport_height:
    g_value_set_int(value, self->renderer_data.viewport_h);
    break;
  case prop_framelimit:
    g_value_set_boolean(value, self->renderer_data.framelimit);
    break;
  case prop_pipeline:
    g_value_set_object(value, self->pipeline);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static
void ds_application_class_init(DsApplicationClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */

  oclass->get_property = ds_application_class_get_property;
  oclass->finalize = ds_application_class_finalize;
  oclass->dispose = ds_application_class_dispose;

/*
 * properties
 *
 */

  properties[prop_dssettings] =
    g_param_spec_object
    (_TRIPLET("dssettings"),
     DS_TYPE_SETTINGS,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_gsettings] =
    g_param_spec_object
    (_TRIPLET("gsettings"),
     G_TYPE_SETTINGS,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_basedatadir] =
    g_param_spec_object
    (_TRIPLET("basedatadir"),
     G_TYPE_FILE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_basecachedir] =
    g_param_spec_object
    (_TRIPLET("basecachedir"),
     G_TYPE_FILE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_savesdir] =
    g_param_spec_object
    (_TRIPLET("savesdir"),
     G_TYPE_FILE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_glcachedir] =
    g_param_spec_object
    (_TRIPLET("glcachedir"),
     G_TYPE_FILE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_lua_state] =
    g_param_spec_pointer
    (_TRIPLET("lua-state"),
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_sdl_window] =
    g_param_spec_pointer
    (_TRIPLET("sdl-window"),
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_gl_context] =
    g_param_spec_pointer
    (_TRIPLET("gl-context"),
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_width] =
    g_param_spec_int
    (_TRIPLET("window-width"),
     0, G_MAXINT,
     0,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_height] =
    g_param_spec_int
    (_TRIPLET("window-height"),
     0, G_MAXINT,
     0,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_viewport_width] =
    g_param_spec_int
    (_TRIPLET("viewport-width"),
     0, G_MAXINT,
     0,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_viewport_height] =
    g_param_spec_int
    (_TRIPLET("viewport-height"),
     0, G_MAXINT,
     0,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_framelimit] =
    g_param_spec_boolean
    (_TRIPLET("framelimit"),
     FALSE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  properties[prop_pipeline] =
    g_param_spec_object
    (_TRIPLET("pipeline"),
     DS_TYPE_PIPELINE,
     G_PARAM_READABLE
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static
void ds_application_init(DsApplication* self) {
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
 * Attach renderer source
 *
 */

  source =
  g_idle_source_new();

  g_source_set_callback
  (source,
   (GSourceFunc)
   _ds_renderer_step,
   g_object_ref(self),
   g_object_unref);

  g_source_set_priority(source, G_PRIORITY_DEFAULT_IDLE);
  g_source_set_name(source, "Renderer source");
  g_source_attach(source, context);
  g_source_unref(source);

/*
 * Attach events source
 *
 */

  source =
  g_idle_source_new();

  g_source_set_callback
  (source,
   (GSourceFunc)
   _ds_events_poll,
   g_object_ref(self),
   g_object_unref);

  g_source_set_priority(source, G_PRIORITY_DEFAULT_IDLE);
  g_source_set_name(source, "Events poll source");
  g_source_attach(source, context);
  g_source_unref(source);

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
  setlocale(LC_ALL, "");

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

  g_signal_connect
  (app,
   "activate",
   G_CALLBACK(on_activate),
   NULL);

  int status =
  g_application_run(app, argc, argv);
  while(G_IS_OBJECT(app))
    g_object_unref(app);
return status;
}
