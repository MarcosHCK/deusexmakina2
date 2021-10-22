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
#ifndef __DS_GAME_OBJECT_INCLUDED__
#define __DS_GAME_OBJECT_INCLUDED__ 1
#include <ds_export.h>
#include <glib-object.h>

#define DS_TYPE_GAME_OBJECT             (ds_game_object_get_type ())
#define DS_GAME_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_GAME_OBJECT, DsGameObject))
#define DS_GAME_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_GAME_OBJECT, DsGameObjectClass))
#define DS_IS_GAME_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_GAME_OBJECT))
#define DS_IS_GAME_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_GAME_OBJECT))
#define DS_GAME_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_GAME_OBJECT, DsGameObjectClass))

typedef struct _DsGameObject        DsGameObject;
typedef struct _DsGameObjectPrivate DsGameObjectPrivate;
typedef struct _DsGameObjectClass   DsGameObjectClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_game_object_get_type ();

/**
 * DsGameObject:
 * @parent_instance: parent instance.
 *
 */
struct _DsGameObject
{
  GObject parent_instance;

  /*<private>*/
  DsGameObjectPrivate* priv;
};

/**
 * DsGameObjectClass:
 * @parent_class: parent class.
 * @collide: emitted when a collision is detected between two object (on both objects)
 *
 */
struct _DsGameObjectClass
{
  GObjectClass parent_class;

  gboolean (*collide) (DsGameObject* this_, DsGameObject* collider, GError** error);
};

DEUSEXMAKINA2_API
gboolean
ds_game_object_collide(DsGameObject* this_, DsGameObject* collider, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_GAME_OBJECT_INCLUDED__
