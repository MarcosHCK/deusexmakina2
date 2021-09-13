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
#ifndef __DS_SETTINGS_INCLUDED__
#define __DS_SETTINGS_INCLUDED__ 1

#define DS_TYPE_SETTINGS            (ds_settings_get_type ())
#define DS_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_SETTINGS, DsSettings))
#define DS_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_SETTINGS, DsSettingsClass))
#define DS_IS_SETTINGS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_SETTINGS))
#define DS_IS_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_SETTINGS))
#define DS_SETTINGS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_SETTINGS, DsSettingsClass))

typedef struct _DsSettings        DsSettings;
typedef struct _DsSettingsClass   DsSettingsClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_settings_get_type();

DsSettings*
ds_settings_new(const gchar* directory,
                GCancellable* cancellable,
                GError** error);
GSettings*
ds_settings_get_settings(DsSettings* self,
                         const gchar* schema_id);
gboolean
ds_settings_has_key(DsSettings* self,
                    const gchar* schema_id,
                    const gchar* key);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_SETTINGS_INCLUDED__
