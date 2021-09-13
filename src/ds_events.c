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
#include <ds_events.h>

gboolean
_ds_events_poll(DsApplication* self)
{
  SDL_Event event;
  lua_State* L = self->L;
  int top = lua_gettop(L);

  while(SDL_PollEvent(&event))
  switch(event.type)
  {
  case SDL_QUIT:
    g_application_quit(G_APPLICATION(self));
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
    ds_events_push(L, argc, &tmp_err);
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

/*
 * init / fini
 *
 */

#define EVENT_PUSH "__DS_EVENT_PUSH"

static int
ticks(lua_State* L) {
  lua_pushnumber
  (L,
   (lua_Number)
   SDL_GetTicks());
return 1;
}

gboolean
_ds_events_init(lua_State      *L,
                GCancellable   *cancellable,
                GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  int top = lua_gettop(L);

/*
 * local event = require('event')
 *
 */

  lua_getglobal(L, "require");
  lua_pushstring(L, "event");

  success =
  ds_xpcall(L, 1, 1, &tmp_err);
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
  lua_settop(L, top);
return success;
}

gboolean
ds_events_push(lua_State  *L,
               int         argc,
               GError    **error)
{
  int top = lua_gettop(L);
  int func = top - argc + 1;

  lua_getfield(L, LUA_REGISTRYINDEX, EVENT_PUSH);
  lua_insert(L, func);

return ds_xpcall(L, argc, 0, error);
}
