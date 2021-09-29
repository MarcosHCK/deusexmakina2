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
#ifndef __DS_RENDER_DATA_INCLUDED__
#define __DS_RENDER_DATA_INCLUDED__
#include <gio/gio.h>

#define DS_TYPE_RENDERER_DATA             (ds_renderer_data_get_type ())
#define DS_RENDERER_DATA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_RENDERER_DATA, DsRendererData))
#define DS_RENDERER_DATA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_RENDERER_DATA, DsRendererDataClass))
#define DS_IS_RENDERER_DATA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_RENDERER_DATA))
#define DS_IS_RENDERER_DATA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_RENDERER_DATA))
#define DS_RENDERER_DATA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_RENDERER_DATA, DsRendererDataClass))

typedef struct _DsRendererData        DsRendererData;
typedef struct _DsRendererDataPrivate DsRendererDataPrivate;
typedef struct _DsRendererDataClass   DsRendererDataClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_renderer_data_get_type();

struct _DsRendererData
{
  GObject parent_instance;
  DsRendererDataPrivate* priv;
  gboolean framelimit;
};

struct _DsRendererDataClass
{
  GObjectClass parent_class;
};

DsRendererData*
ds_renderer_data_new(GSettings     *gsettings,
                     gpointer       window,
                     GCancellable  *cancellable,
                     GError       **error);
void
ds_renderer_data_force_update(DsRendererData* self);
void
ds_renderer_data_look(DsRendererData *data,
                      gfloat          x,
                      gfloat          y,
                      gfloat          xrel,
                      gfloat          yrel);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_RENDER_DATA_INCLUDED__
