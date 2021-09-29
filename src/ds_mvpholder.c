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
#include <cglm/cglm.h>
#include <ds_mvpholder.h>

static
void ds_mvp_holder_default_init(DsMvpHolderIface* iface);

/*
 * Object definition
 *
 */

GType
ds_mvp_holder_get_type(void)
{
  static
  GType holder_type = 0;

  if G_UNLIKELY(holder_type == 0)
  {
    static const
    GTypeInfo type_info = {
      sizeof(DsMvpHolderIface),
      NULL,
      NULL,
      (GClassInitFunc)
      ds_mvp_holder_default_init,
      NULL,
      NULL,
      0,
      0,
      NULL
    };

    holder_type =
    g_type_register_static
    (G_TYPE_INTERFACE,
     g_intern_static_string
     ("DsMvpHolder"),
     &type_info,
     0);

    g_type_interface_add_prerequisite
    (holder_type,
     G_TYPE_OBJECT);
  }
return holder_type;
}

static
void ds_mvp_holder_default_init(DsMvpHolderIface* iface)
{
}

/*
 * Interface methods
 *
 */

void
ds_mvp_holder_set_model(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_model == 0)
  {
    g_warning
    ("DsRenderable::p_model unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_model;
  glm_mat4_copy((gpointer) mat4_, ptr);

  if(iface->notify_model != NULL)
  {
    iface->notify_model(holder);
  }
}

void
ds_mvp_holder_get_model(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_model == 0)
  {
    g_warning
    ("DsRenderable::p_model unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_model;
  glm_mat4_copy(ptr, (gpointer) mat4_);
}

void
ds_mvp_holder_set_view(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_view == 0)
  {
    g_warning
    ("DsRenderable::p_view unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_view;
  glm_mat4_copy((gpointer) mat4_, ptr);

  if(iface->notify_view != NULL)
  {
    iface->notify_view(holder);
  }
}

void
ds_mvp_holder_get_view(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_view == 0)
  {
    g_warning
    ("DsRenderable::p_view unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_view;
  glm_mat4_copy(ptr, (gpointer) mat4_);
}

void
ds_mvp_holder_set_projection(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_projection == 0)
  {
    g_warning
    ("DsRenderable::p_projection unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_projection;
  glm_mat4_copy((gpointer) mat4_, ptr);

  if(iface->notify_projection != NULL)
  {
    iface->notify_projection(holder);
  }
}

void
ds_mvp_holder_get_projection(DsMvpHolder* holder, gfloat* mat4_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(mat4_ != NULL);
  DsMvpHolderIface* iface = DS_MVP_HOLDER_GET_INTERFACE(holder);

  if G_UNLIKELY
    (iface->p_projection == 0)
  {
    g_warning
    ("DsRenderable::p_projection unset for '%s'\r\n",
     g_type_name(G_TYPE_FROM_INSTANCE(holder)));
    return;
  }

  gpointer ptr  = (gpointer) holder;
           ptr += iface->p_projection;
  glm_mat4_copy(ptr, (gpointer) mat4_);
}

void
ds_mvp_holder_set_position(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->set_position(holder, vec3_);
}

void
ds_mvp_holder_get_position(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->get_position(holder, vec3_);
}
