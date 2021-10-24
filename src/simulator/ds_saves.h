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
#ifndef __DS_SAVES_INCLUDED__
#define __DS_SAVES_INCLUDED__ 1
#include <gio/gio.h>

#define DS_TYPE_SAVE            (ds_save_get_type())
#define DS_SAVE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DS_TYPE_SAVE, DsSave))
#define DS_SAVE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), DS_TYPE_SAVE, DsSaveClass))
#define DS_IS_SAVE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), DS_TYPE_SAVE))
#define DS_IS_SAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), DS_TYPE_SAVE))
#define DS_SAVE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), DS_TYPE_SAVE, DsSaveClass))

typedef struct _DsSave      DsSave;
typedef struct _DsSaveClass DsSaveClass;

#define DS_AUTOSAVE "autosave"

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_save_get_type();

DsSave*
ds_save_new(const gchar* name,
            GFile* savesdir,
            GCancellable* cancellable,
            GError** error);
GOutputStream*
ds_save_save(DsSave* self,
             GCancellable* cancellable,
             GError** error);
GInputStream*
ds_save_load(DsSave* self,
             GCancellable* cancellable,
             GError** error);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(DsSave, g_object_unref)

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_SAVES_INCLUDED__
