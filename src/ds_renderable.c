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
#include <ds_renderable.h>

static
void ds_renderable_default_init(DsRenderableIface* iface);

/*
 * Interface definition
 *
 */

GType
ds_renderable_get_type (void)
{
  static
  GType renderable_type = 0;

  if G_UNLIKELY(renderable_type == 0)
  {
    const
    GTypeInfo type_info = {
      sizeof(DsRenderableIface),
      NULL,
      NULL,
      (GClassInitFunc)
      ds_renderable_default_init,
      NULL,
      NULL,
      0,
      0,
      NULL
    };

    renderable_type =
    g_type_register_static
    (G_TYPE_INTERFACE,
     "DsRenderable",
     &type_info,
     0);
  }
return renderable_type;
}

static
void ds_renderable_default_init(DsRenderableIface* iface) {
}

/*
 * Object methods
 *
 */
