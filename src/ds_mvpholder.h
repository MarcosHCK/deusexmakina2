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
#ifndef __DS_MVPHOLDER_INCLUDED__
#define __DS_MVPHOLDER_INCLUDED__ 1
#include <ds_export.h>
#include <glib-object.h>

#define DS_TYPE_MVP_HOLDER                (ds_mvp_holder_get_type ())
#define DS_MVP_HOLDER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_MVP_HOLDER, DsMvpHolder))
#define DS_IS_MVP_HOLDER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_MVP_HOLDER))
#define DS_MVP_HOLDER_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), DS_TYPE_MVP_HOLDER, DsMvpHolderIface))

typedef struct _DsMvpHolder             DsMvpHolder;
typedef struct _DsMvpHolderIface        DsMvpHolderIface;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_mvp_holder_get_type();

/**
 * DsMvpHolderIface:
 * @parent_iface: parent type data.
 * @p_model: offset into instance structure which holds 'model' matrix.
 * @p_view: offset into instance structure which holds 'view' matrix.
 * @p_projection: offset into instance structure which holds 'projection' matrix.
 * @notify_model: a callback to be called when 'model' matrix if changed.
 * @notify_view: a callback to be called when 'view' matrix if changed.
 * @notify_projection: a callback to be called when 'projection' matrix if changed.
 * @set_position: virtual function for #ds_mvp_holder_set_position().
 * @get_position: virtual function for #ds_mvp_holder_get_position().
 * @set_scale: virtual function for #ds_mvp_holder_set_scale().
 * @get_scale: virtual function for #ds_mvp_holder_get_scale().
 *
 * The #DsMvpHolderIface interface defines a fast access method
 * to modify MVP matrix components implemented be a #DsMvpHolder
 * instance.
 *
 */
struct _DsMvpHolderIface
{
  GTypeInterface parent_iface;

  goffset p_model;
  goffset p_view;
  goffset p_projection;

  void (*notify_model) (DsMvpHolder* holder);
  void (*notify_view) (DsMvpHolder* holder);
  void (*notify_projection) (DsMvpHolder* holder);

  void (*set_position) (DsMvpHolder* holder, gfloat* vec3_);
  void (*get_position) (DsMvpHolder* holder, gfloat* vec3_);
  void (*set_scale) (DsMvpHolder* holder, gfloat* vec3_);
  void (*get_scale) (DsMvpHolder* holder, gfloat* vec3_);
};

DEUSEXMAKINA2_API
void
ds_mvp_holder_set_model(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_get_model(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_set_view(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_get_view(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_set_projection(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_get_projection(DsMvpHolder* holder, gfloat* mat4_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_set_position(DsMvpHolder* holder, gfloat* vec3_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_get_position(DsMvpHolder* holder, gfloat* vec3_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_set_scale(DsMvpHolder* holder, gfloat* vec3_);
DEUSEXMAKINA2_API
void
ds_mvp_holder_get_scale(DsMvpHolder* holder, gfloat* vec3_);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_MVPHOLDER_INCLUDED__
