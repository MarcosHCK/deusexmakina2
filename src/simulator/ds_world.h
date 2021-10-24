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
#ifndef __DS_WORLD_INCLUDED__
#define __DS_WORLD_INCLUDED__ 1
#include <ds_export.h>
#include <glib-object.h>

#define DS_TYPE_WORLD             (ds_world_get_type ())
#define DS_WORLD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_WORLD, DsWorld))
#define DS_WORLD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_WORLD, DsWorldClass))
#define DS_IS_WORLD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_WORLD))
#define DS_IS_WORLD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_WORLD))
#define DS_WORLD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_WORLD, DsWorldClass))

typedef struct _DsWorld         DsWorld;
typedef struct _DsWorldPrivate  DsWorldPrivate;
typedef struct _DsWorldClass    DsWorldClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

DEUSEXMAKINA2_API
GType
ds_world_get_type();

struct _DsWorld
{
  GObject parent_instance;
  DsWorldPrivate* priv;
};

struct _DsWorldClass
{
  GObjectClass parent_class;
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_WORLD_INCLUDED__
