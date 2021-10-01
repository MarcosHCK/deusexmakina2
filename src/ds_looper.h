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
#ifndef __DS_LOOPER_INCLUDED__
#define __DS_LOOPER_INCLUDED__ 1
#include <glib-object.h>

#define DS_TYPE_LOOPER            (ds_looper_get_type ())
#define DS_LOOPER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_LOOPER, DsLooper))
#define DS_LOOPER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_LOOPER, DsLooperClass))
#define DS_IS_LOOPER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_LOOPER))
#define DS_IS_LOOPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_LOOPER))
#define DS_LOOPER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_LOOPER, DsLooperClass))

typedef struct _DsLooper        DsLooper;
typedef struct _DsLooperClass   DsLooperClass;
typedef struct _DsLooperPrivate DsLooperPrivate;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_looper_get_type();

struct _DsLooper
{
  GObject parent_instance;
  DsLooperPrivate * priv;
};

struct _DsLooperClass
{
  GObjectClass parent_class;
  gboolean (*loop_step) (DsLooper* self);
};

void ds_looper_start (DsLooper* self);
void ds_looper_stop (DsLooper* self);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LOOPER_INCLUDED__
