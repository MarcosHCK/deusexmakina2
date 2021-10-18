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
#ifndef __LUAD_GIRHUB_INCLUDED__
#define __LUAD_GIRHUB_INCLUDED__ 1
#include <ds_export.h>
#include <glib-object.h>
#include <luad_core.h>

#define LUAD_TYPE_GIR_HUB             (luad_gir_hub_get_type ())
#define LUAD_GIR_HUB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUAD_TYPE_GIR_HUB, luaDGirHub))
#define LUAD_GIR_HUB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUAD_TYPE_GIR_HUB, luaDGirHubClass))
#define LUAD_IS_GIR_HUB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUAD_TYPE_GIR_HUB))
#define LUAD_IS_GIR_HUB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUAD_TYPE_GIR_HUB))
#define LUAD_GIR_HUB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUAD_TYPE_GIR_HUB, luaDGirHubClass))

typedef struct _luaDGirHub        luaDGirHub;
typedef struct _luaDGirHubClass   luaDGirHubClass;
typedef struct _luaDGirHubPrivate luaDGirHubPrivate;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
luad_gir_hub_get_type ();

G_GNUC_INTERNAL
gboolean
_luaD_girhub_init (lua_State* L, GError** error);
G_GNUC_INTERNAL
void
_luaD_girhub_fini (lua_State* L);

luaDGirHub*
luaD_gir_hub_new (void);
luaDGirHub*
luaD_gir_hub_get_default (void);

#if __cplusplus
}
#endif // __cplusplus

#endif // __LUAD_GIRHUB_INCLUDED__
