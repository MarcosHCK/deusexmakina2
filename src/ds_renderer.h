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
#ifndef __DS_RENDERER_INCLUDED__
#define __DS_RENDERER_INCLUDED__ 1
#include <ds_export.h>
#include <gio/gio.h>

#define DS_TYPE_RENDERER            (ds_renderer_get_type())
#define DS_RENDERER(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), DS_TYPE_RENDERER, DsRenderer))
#define DS_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_RENDERER, DsRendererClass))
#define DS_IS_RENDERER(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), DS_TYPE_RENDERER))
#define DS_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_RENDERER))
#define DS_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_RENDERER, DsRendererClass))

typedef struct _DsRenderer      DsRenderer;
typedef struct _DsRendererClass DsRendererClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_renderer_get_type();

DEUSEXMAKINA2_API
DsRenderer*
ds_renderer_new(GSettings      *gsettings,
                gpointer        pipeline,
                gpointer        window,
                GCancellable   *cancellable,
                GError        **error);
DEUSEXMAKINA2_API
void
ds_renderer_force_update(DsRenderer* renderer);
DEUSEXMAKINA2_API
void
ds_renderer_look(DsRenderer  *renderer,
                 gfloat       xrel,
                 gfloat       yrel);
DEUSEXMAKINA2_API
void
ds_renderer_move(DsRenderer  *renderer,
                 gfloat       xrel,
                 gfloat       yrel,
                 gfloat       zrel,
                 gboolean     relative);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_RENDERER_INCLUDED__
