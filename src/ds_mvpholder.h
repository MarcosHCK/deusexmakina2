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
#include <cglm/cglm.h>
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

GType
ds_mvp_holder_get_type();

struct _DsMvpHolderIface
{
  GTypeInterface parent_iface;
  goffset p_model;
  goffset p_view;
  goffset p_projection;
};

void
ds_mvp_holder_set_model(DsMvpHolder* holder, gfloat* mat4_);
void
ds_mvp_holder_get_model(DsMvpHolder* holder, gfloat* mat4_);
void
ds_mvp_holder_set_view(DsMvpHolder* holder, gfloat* mat4_);
void
ds_mvp_holder_get_view(DsMvpHolder* holder, gfloat* mat4_);
void
ds_mvp_holder_set_projection(DsMvpHolder* holder, gfloat* mat4_);
void
ds_mvp_holder_get_projection(DsMvpHolder* holder, gfloat* mat4_);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_MVPHOLDER_INCLUDED__
