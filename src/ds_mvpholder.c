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

/**
 * SECTION:dsmvpholder
 * @Short_description: MVP matrix components holder
 * @Title: DsMvpHolder
 *
 * DsMvpHolder is an interface to access standard MVP
 * (model-view-projection) matrix components:
 *
 * - 'model': stores model-specific transformations.
 * - 'view': stores camera (or eye) transformations.
 * - 'projection': stores projection (or clip) transformations.
 *
 * DsMvpHolder also implements some other convenience functions
 * for more abstract concepts, such as positions or scales.
 *
 */

/*
 * Object definition
 *
 */

typedef DsMvpHolderIface DsMvpHolderInterface;
G_DEFINE_INTERFACE(DsMvpHolder, ds_mvp_holder, G_TYPE_OBJECT);

static void
ds_mvp_holder_default_init(DsMvpHolderIface* iface)
{
}

/*
 * Interface methods
 *
 */

/**
 * ds_mvp_holder_set_model:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @mat4_ to @holder internal matrix store.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'model' store this call has not effect.
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

/**
 * ds_mvp_holder_get_model:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @holder internal matrix store to @mat4_.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'model' store this call has not effect.
 *
 */
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

/**
 * ds_mvp_holder_set_view:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @mat4_ to @holder internal matrix store.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'view' store this call has not effect.
 *
 */
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

/**
 * ds_mvp_holder_get_view:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @holder internal matrix store to @mat4_.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'view' store this call has not effect.
 *
 */
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

/**
 * ds_mvp_holder_set_projection:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @mat4_ to @holder internal matrix store.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'projection' store this call has not effect.
 *
 */
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

/**
 * ds_mvp_holder_get_projection:
 * @holder: a #DsMvpHolder instance.
 * @mat4_: an address to a 4x4 #gfloat matrix.
 *
 * Copies @holder internal matrix store to @mat4_.
 * Note that not all #DsMvpHolder may implement a
 * all stores, and in case of @holder doesn't implements
 * 'projection' store this call has not effect.
 *
 */
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

/**
 * ds_mvp_holder_set_position:
 * @holder: a #DsMvpHolder instance.
 * @vec3_: an address to a #gfloat 3-dimensions vector
 *
 * Sets @holder position if positions makes sense for it.
 * Note: this feature is optional because not all objects
 * understand the concept of position.
 *
 */
void
ds_mvp_holder_set_position(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->set_position(holder, vec3_);
}

/**
 * ds_mvp_holder_get_position:
 * @holder: a #DsMvpHolder instance.
 * @vec3_: an address to a #gfloat 3-dimensions vector
 *
 * Gets @holder position if positions makes sense for it.
 * Note: this feature is optional because not all objects
 * understand the concept of position.
 *
 */
void
ds_mvp_holder_get_position(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->get_position(holder, vec3_);
}

/**
 * ds_mvp_holder_set_scale:
 * @holder: a #DsMvpHolder instance.
 * @vec3_: an address to a #gfloat 3-dimensions vector
 *
 * Sets @holder scale if positions makes sense for it.
 * Note: this feature is optional because not all objects
 * understand the concept of scale.
 *
 */
void
ds_mvp_holder_set_scale(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->set_scale(holder, vec3_);
}

/**
 * ds_mvp_holder_get_scale:
 * @holder: a #DsMvpHolder instance.
 * @vec3_: an address to a #gfloat 3-dimensions vector
 *
 * Gets @holder scale if positions makes sense for it.
 * Note: this feature is optional because not all objects
 * understand the concept of scale.
 *
 */
void
ds_mvp_holder_get_scale(DsMvpHolder* holder, gfloat* vec3_)
{
  g_return_if_fail(DS_IS_MVP_HOLDER(holder));
  g_return_if_fail(vec3_ != NULL);

  DsMvpHolderIface* iface =
  DS_MVP_HOLDER_GET_INTERFACE(holder);
  iface->get_scale(holder, vec3_);
}
