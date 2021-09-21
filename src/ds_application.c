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
#include <ds_luaobj.h>
#include <ds_model.h>
#include <ds_renderer.h>
#include <ds_skybox.h>
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
_IMG_fini0(guint var)
{
  (var == 0) ? 0 : (var = (IMG_Quit (), 0));
}

static void
_TTF_fini0(guint var)
{
  (var == 0) ? 0 : (var = (TTF_Quit (), 0));
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

G_DEFINE_TYPE_WITH_CODE
(DsApplication,
 ds_application,
 G_TYPE_APPLICATION,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_application_g_initiable_iface_init));

static gboolean
ds_application_g_initiable_iface_init_sync(GInitable     *pself,
                                           GCancellable  *cancellable,
                                           GError       **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsApplication* self =
  DS_APPLICATION(pself);
  gint return_ = 0;

  DsSettings* dssettings = NULL;
  GSettings* gsettings = NULL;
  GFile* basedatadir = NULL;
  GFile* savesdir = NULL;
  lua_State* L = NULL;
  guint sdl_init = 0;
  guint img_init = 0;
  guint ttf_init = 0;
  SDL_Window* window = NULL;
  SDL_GLContext* glctx = NULL;
  guint glew_init = 0;
  DsPipeline* pipeline = NULL;

/*
 * Ensure class initialization
 *
 */

  g_type_ensure(DS_TYPE_APPLICATION);
  g_type_ensure(DS_TYPE_ERROR);
  g_type_ensure(DS_TYPE_MODEL);
  g_type_ensure(DS_TYPE_PIPELINE);
  g_type_ensure(DS_TYPE_SAVE);
  g_type_ensure(DS_TYPE_SETTINGS);
  g_type_ensure(DS_TYPE_SHADER);
  g_type_ensure(DS_TYPE_SKYBOX);

/*
 * Settings
 *
 */

  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
    dssettings =
    ds_settings_new
    (ABSTOPBUILDDIR
     "/settings/",
     cancellable,
     &tmp_err);
  else
    dssettings =
    ds_settings_new
    (SCHEMASDIR,
     cancellable,
     &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  const gchar* schema_id = GAPPNAME;

  gsettings =
  ds_settings_get_settings
  (dssettings,
   schema_id);

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

  savesdir =
  _ds_base_data_dir_child("saves", basedatadir, cancellable, &tmp_err);
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

  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
    return_ =
    luaL_loadfile
    (L,
     ABSTOPBUILDDIR
     "/scripts/"
     "patches.lua");
    lua_pushstring
    (L,
     ABSTOPBUILDDIR
     "/scripts/");
  }
  else
  {
    return_ =
    luaL_loadfile
    (L,
     PKGLIBEXECDIR
     "/patches.lua");
    lua_pushstring
    (L,
     PKGLIBEXECDIR);
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
  ds_xpcall(L, 1, 0, &tmp_err);
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

  return_ =
  IMG_Init(IMG_INIT_WEBP);
  if G_UNLIKELY(0 > return_)
  {
    const gchar* err =
    SDL_GetError();
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_SDL2_INIT,
     "IMG_Init(): failed!: %i: %s\r\n",
     return_, err ? err : "(null)");
    goto_error();
  }
  else
  {
    img_init = 1;
  }

  return_ =
  TTF_Init();
  if G_UNLIKELY(0 > return_)
  {
    const gchar* err =
    SDL_GetError();
    g_set_error
    (error,
     DS_APPLICATION_ERROR,
     DS_APPLICATION_ERROR_SDL2_INIT,
     "TTF_Init(): failed!: %i: %s\r\n",
     return_, err ? err : "(null)");
    goto_error();
  }
  else
  {
    ttf_init = 1;
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

  g_settings_get
  (gsettings,
   "fullscreen",
   "b", &fullscreen);
  g_settings_get
  (gsettings,
   "borderless",
   "b", &borderless);

  if(fullscreen == TRUE)
    flags |= SDL_WINDOW_FULLSCREEN;
  else
  if(borderless == TRUE)
    flags |= SDL_WINDOW_BORDERLESS;

  g_settings_get
  (gsettings,
   "width",
   "i", &width);
  g_settings_get
  (gsettings,
   "height",
   "i", &height);

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

  self->width = width;
  self->height = height;

  g_settings_get
  (gsettings,
   "framelimit",
   "b",
   &(self->framelimit));

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
   &(self->viewport_w),
   &(self->viewport_h));

  SDL_ShowCursor(1);

/*
 * GLEW
 *
 */

  return_ =
  glewInit();
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

  /* set projection matrix */
  _ds_application_update_projection(self, pipeline);
  _ds_application_update_view(self, pipeline, 0.f, 0.f, 0.f, 0.f);

/*
 * Execute setup script
 *
 */

  if G_UNLIKELY
    (g_strcmp0
     (g_getenv("DS_DEBUG"),
      "true") == 0)
  {
    return_ =
    luaL_loadfile
    (L,
     ABSTOPBUILDDIR
     "/scripts/"
     "setup.lua");
  }
  else
  {
    return_ =
    luaL_loadfile
    (L,
     PKGLIBEXECDIR
     "/setup.lua");
  }

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

  if(cancellable != NULL)
    luaD_pushobject(L, G_OBJECT(cancellable));
  else
    lua_pushnil(L);

  success =
  ds_xpcall(L, 1, 0, &tmp_err);
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
 * All goes fine
 *
 */

  self->dssettings = g_steal_pointer(&dssettings);
  self->gsettings = g_steal_pointer(&gsettings);
  self->basedatadir = g_steal_pointer(&basedatadir);
  self->savesdir = g_steal_pointer(&savesdir);
  self->L = g_steal_pointer(&L);
  self->sdl_init = ds_steal_handle_id(&sdl_init);
  self->img_init = ds_steal_handle_id(&img_init);
  self->ttf_init = ds_steal_handle_id(&ttf_init);
  self->window = g_steal_pointer(&window);
  self->glctx = g_steal_pointer(&glctx);
  self->glew_init = ds_steal_handle_id(&glew_init);
  self->pipeline = g_steal_pointer(&pipeline);
  lua_settop(self->L, 0);

_error_:
  g_clear_object(&pipeline);
  g_clear_handle_id
  (&glew_init, _glew_fini0);
  g_clear_pointer
  (&glctx, _SDL_GL_DeleteContext0);
  g_clear_pointer
  (&window, _SDL_DestroyWindow0);
  g_clear_handle_id
  (&ttf_init, _TTF_fini0);
  g_clear_handle_id
  (&img_init, _IMG_fini0);
  g_clear_handle_id
  (&sdl_init, _SDL_fini0);
  g_clear_pointer
  (&L, _lua_close0);
  g_clear_object(&savesdir);
  g_clear_object(&basedatadir);
  g_clear_object(&gsettings);
  g_clear_object(&dssettings);
return success;
}

static
void ds_application_g_initiable_iface_init(GInitableIface* iface) {
  iface->init = ds_application_g_initiable_iface_init_sync;
}

static
void ds_application_class_finalize(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);

/*
 * Finalize
 *
 */
  _glew_fini0(self->glew_init);
  _SDL_GL_DeleteContext0(self->glctx);
  _SDL_DestroyWindow0(self->window);
  _TTF_fini0(self->ttf_init);
  _IMG_fini0(self->img_init);
  _SDL_fini0(self->sdl_init);
  _lua_close0(self->L);

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(ds_application_parent_class)->finalize(pself);
}

static
void ds_application_class_dispose(GObject* pself) {
  DsApplication* self = DS_APPLICATION(pself);

/*
 * Dispose
 *
 */
  g_clear_object(&(self->pipeline));
  g_clear_object(&(self->savesdir));
  g_clear_object(&(self->basedatadir));
  g_clear_object(&(self->gsettings));
  g_clear_object(&(self->dssettings));

/*
 * Chain-up
 *
 */
  G_OBJECT_CLASS(ds_application_parent_class)->dispose(pself);
}

static
void ds_application_class_init(DsApplicationClass* klass) {
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

/*
 * vtable
 *
 */
  oclass->finalize = ds_application_class_finalize;
  oclass->dispose = ds_application_class_dispose;
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
  GError* tmp_err = NULL;
  GApplication* app =
  (GApplication*)
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

  g_signal_connect_data
  (app,
   "activate",
   G_CALLBACK(on_activate),
   NULL, NULL,
   0);

  int status =
  g_application_run(app, argc, argv);
  while(G_IS_OBJECT(app))
    g_object_unref(app);
return status;
}
