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
#include <ds_events.h>
#include <ds_looper.h>
#include <ds_macros.h>
#include <luad_core.h>
#include <SDL.h>

/**
 * SECTION:dsevents
 * @Short_description: events controller
 * @Title: DsEvents
 *
 * DsEvents encapsulates complexities of
 * launching SDL events into Lua space
 *
 */

#define EVENT_PUSH "__DS_EVENT_PUSH"

static void
ds_events_g_initable_iface_init(GInitableIface* iface);

/*
 * Object definition
 *
 */

struct _DsEvents
{
  DsLooper parent_instance;

  /*<private>*/
  lua_State* L;
};

struct _DsEventsClass
{
  DsLooperClass parent_class;
};

enum
{
  prop_0,
  prop_engine,
  prop_number,
};

static
GParamSpec* properties[prop_number] = {0};

G_DEFINE_TYPE_WITH_CODE
(DsEvents,
 ds_events,
 DS_TYPE_LOOPER,
 G_IMPLEMENT_INTERFACE
 (G_TYPE_INITABLE,
  ds_events_g_initable_iface_init));

static int
ticks(lua_State* L) {
  lua_pushnumber
  (L,
   (lua_Number)
   SDL_GetTicks());
return 1;
}

static gboolean
ds_events_g_initable_iface_init_sync(GInitable* pself, GCancellable* cancellable, GError** error)
{
  DsEvents* self = DS_EVENTS(pself);
  lua_State* L = self->L;
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 * local event = require('event')
 *
 */

  lua_getglobal(L, "require");
  lua_pushstring(L, "event");

  success =
  luaD_xpcall(L, 1, 1, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * event.ticks = C(ticks)
 *
 */

  lua_pushstring(L, "ticks");
  lua_pushcfunction(L, ticks);
  lua_settable(L, -3);

/*
 * _R['__DS_EVENT_PUSH'] = event.push
 *
 */

  lua_pushstring(L, "push");
  lua_gettable(L, -2);

  lua_setfield(L, LUA_REGISTRYINDEX, EVENT_PUSH);
_error_:
return success;
}

static void
ds_events_g_initable_iface_init(GInitableIface* iface)
{
  iface->init = ds_events_g_initable_iface_init_sync;
}

static gboolean
push_event(lua_State* L, int argc, GError** error)
{
  int top = lua_gettop(L);
  int func = top - argc + 1;

  lua_getfield(L, LUA_REGISTRYINDEX, EVENT_PUSH);
  lua_insert(L, func);
return luaD_xpcall(L, argc, 0, error);
}

static gboolean
ds_events_class_loop_step(DsLooper* pself)
{
  DsEvents* self = DS_EVENTS(pself);
  lua_State* L = self->L;
  int top = lua_gettop(L);

  SDL_Event event;
  while(SDL_PollEvent(&event))
  switch(event.type)
  {
  case SDL_QUIT:
    g_application_quit(g_application_get_default());
    break;
  case SDL_MOUSEMOTION:
    lua_pushstring(L, "mouse_motion");
    lua_pushnumber(L, event.motion.x);
    lua_pushnumber(L, event.motion.y);
    lua_pushnumber(L, event.motion.xrel);
    lua_pushnumber(L, event.motion.yrel);
    break;
  case SDL_MOUSEWHEEL:
    lua_pushstring(L, "mouse_wheel");
    lua_pushnumber(L, event.wheel.x);
    lua_pushnumber(L, event.wheel.y);
    lua_pushnumber(L, event.wheel.direction);
    break;
  case SDL_MOUSEBUTTONDOWN:
    lua_pushstring(L, "mouse_button_down");
    lua_pushnumber(L, event.button.x);
    lua_pushnumber(L, event.button.y);
    lua_pushnumber(L, event.button.button);
    lua_pushnumber(L, event.button.clicks);
    break;
  case SDL_MOUSEBUTTONUP:
    lua_pushstring(L, "mouse_button_up");
    lua_pushnumber(L, event.button.x);
    lua_pushnumber(L, event.button.y);
    lua_pushnumber(L, event.button.button);
    lua_pushnumber(L, event.button.clicks);
    break;
  case SDL_KEYUP:
    lua_pushstring(L, "key_up");
    lua_pushnumber(L, event.key.keysym.sym);
    lua_pushnumber(L, event.key.repeat);
    lua_pushboolean(L, event.key.state);
    break;
  case SDL_KEYDOWN:
    lua_pushstring(L, "key_down");
    lua_pushnumber(L, event.key.keysym.sym);
    lua_pushnumber(L, event.key.repeat);
    lua_pushboolean(L, event.key.state);
    break;
  }

  int argc = lua_gettop(L) - top;
  if G_LIKELY(argc > 0)
  {
    GError* tmp_err = NULL;

    push_event(L, argc, &tmp_err);
    if G_UNLIKELY(tmp_err != NULL)
    {
      g_critical(tmp_err->message);
      g_clear_error(&tmp_err);
      g_assert_not_reached();
    }
  }

  lua_settop(L, top);
return G_SOURCE_CONTINUE;
}

static void
ds_events_class_set_property(GObject* pself, guint prop_id, const GValue* value, GParamSpec* pspec)
{
  DsEvents* self = DS_EVENTS(pself);
  switch(prop_id)
  {
  case prop_engine:
    self->L = g_value_get_pointer(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(pself, prop_id, pspec);
    break;
  }
}

static void
ds_events_class_dispose(GObject* pself)
{
  DsEvents* self = DS_EVENTS(pself);
G_OBJECT_CLASS(ds_events_parent_class)->dispose(pself);
}

static void
ds_events_class_init(DsEventsClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);
  DsLooperClass* lclass = DS_LOOPER_CLASS(klass);

  lclass->loop_step = ds_events_class_loop_step;

  oclass->set_property = ds_events_class_set_property;
  oclass->dispose = ds_events_class_dispose;

  properties[prop_engine] =
    g_param_spec_pointer
    (_TRIPLET("engine"),
     G_PARAM_WRITABLE
     | G_PARAM_CONSTRUCT_ONLY
     | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties
  (oclass,
   prop_number,
   properties);
}

static void
ds_events_init(DsEvents* self)
{
}

/*
 * Object methods
 *
 */

/**
 * ds_events_new: (constructor)
 * @engine: (not nullable): Lua state.
 * @cancellable: (nullable): a %GCancellable
 * @error: return location for a #GError
 *
 * Creates a new #DsEvents which operates on
 * @engine Lua state.
 *
 * Returns: (transfer full): see description.
 */
DsEvents*
ds_events_new(gpointer        engine,
              GCancellable   *cancellable,
              GError        **error)
{
  return (DsEvents*)
  g_initable_new
  (DS_TYPE_EVENTS,
   cancellable,
   error,
   "engine", engine,
   NULL);
}
