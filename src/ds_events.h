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
#define __DS_EVENTS_INCLUDED__ 1
#include <ds_export.h>
#include <gio/gio.h>

#define DS_TYPE_EVENTS            (ds_events_get_type())
#define DS_EVENTS(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_EVENTS, DsEvents))
#define DS_EVENTS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_EVENTS, DsEventsClass))
#define DS_IS_EVENTS(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_EVENTS))
#define DS_IS_EVENTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_EVENTS))
#define DS_EVENTS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_EVENTS, DsEventsClass))

typedef struct _DsEvents      DsEvents;
typedef struct _DsEventsClass DsEventsClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_events_get_type();

DEUSEXMAKINA2_API
DsEvents*
ds_events_new(GSettings      *gsettings,
              gpointer        window,
              GCancellable   *cancellable,
              GError        **error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_EVENTS_INCLUDED__
