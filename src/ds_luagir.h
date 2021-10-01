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
#ifndef __DS_LUAGIR_INCLUDED__
#define __DS_LUAGIR_INCLUDED__
#include <ds_closure.h>
#include <ds_lua.h>

#define DS_TYPE_GIR_HUB             (ds_gir_hub_get_type ())
#define DS_GIR_HUB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DS_TYPE_GIR_HUB, DsGirHub))
#define DS_GIR_HUB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DS_TYPE_GIR_HUB, DsGirHubClass))
#define DS_IS_GIR_HUB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DS_TYPE_GIR_HUB))
#define DS_IS_GIR_HUB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DS_TYPE_GIR_HUB))
#define DS_GIR_HUB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DS_TYPE_GIR_HUB, DsGirHubClass))

typedef struct _DsGirHub        DsGirHub;
typedef struct _DsGirHubClass   DsGirHubClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
ds_gir_hub_get_type();

DsGirHub*
ds_gir_hub_get_default(void);
DsClosure*
ds_gir_hub_get_method(DsGirHub     *self,
                      GType         g_type,
                      const gchar  *name,
                      GError      **error);

gboolean
_ds_luagir_init(lua_State  *L,
                GError    **error);
void
_ds_luagir_fini(lua_State* L);

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_LUAGIR_INCLUDED__
