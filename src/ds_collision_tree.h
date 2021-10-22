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
#ifndef __DS_COLLISION_TREE_INCLUDED__
#define __DS_COLLISION_TREE_INCLUDED__
#include <ds_export.h>
#include <glib-object.h>

#define DS_TYPE_COLLISION_TREE            (ds_collision_tree_get_type ())
#define DS_COLLISION_TREE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_COLLISION_TREE, DsCollisionTree))
#define DS_COLLISION_TREE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_COLLISION_TREE, DsCollisionTreeClass))
#define DS_IS_COLLISION_TREE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_COLLISION_TREE))
#define DS_IS_COLLISION_TREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_COLLISION_TREE))
#define DS_COLLISION_TREE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_COLLISION_TREE, DsCollisionTreeClass))

typedef struct _DsCollisionTree       DsCollisionTree;
typedef struct _DsCollisionTreeClass  DsCollisionTreeClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_collision_tree_get_type();

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_COLLISION_TREE_INCLUDED__
