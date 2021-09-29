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
#ifndef __DS_EVENTS_INCLUDED__
#define __DS_EVENTS_INCLUDED__
#include <gio/gio.h>
#include <ds_application.h>
#include <ds_lua.h>
#include <ds_renderer_data.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL
gboolean
_ds_events_init(lua_State      *L,
                GSettings      *settings,
                GCancellable   *cancellable,
                GError        **error);
G_GNUC_INTERNAL
gboolean
_ds_events_poll(DsRendererData* data);
G_GNUC_INTERNAL
gboolean
ds_events_push(lua_State  *L,
               int         argc,
               GError    **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_EVENTS_INCLUDED__
