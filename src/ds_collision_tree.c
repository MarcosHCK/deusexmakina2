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
#include <ds_collision_tree.h>

/*
 * Object definition
 *
 */

struct _DsCollisionTree
{
  GObject parent_instance;

  /*<private>*/
};

struct _DsCollisionTreeClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE_WITH_CODE
(DsCollisionTree,
 ds_collision_tree,
 G_TYPE_OBJECT,);

static void
ds_collision_tree_class_finalize(GObject* pself)
{
  DsCollisionTree* self = DS_COLLISION_TREE(pself);
G_OBJECT_CLASS(ds_collision_tree_parent_class)->finalize(pself);
}

static void
ds_collision_tree_class_dispose(GObject* pself)
{
  DsCollisionTree* self = DS_COLLISION_TREE(pself);
G_OBJECT_CLASS(ds_collision_tree_parent_class)->dispose(pself);
}

static void
ds_collision_tree_class_init(DsCollisionTreeClass* klass)
{
  GObjectClass* oclass = G_OBJECT_CLASS(klass);

  oclass->finalize = ds_collision_tree_class_finalize;
  oclass->dispose = ds_collision_tree_class_dispose;
}

static void
ds_collision_tree_init(DsCollisionTree* self)
{
}

/*
 * Object methods
 *
 */
