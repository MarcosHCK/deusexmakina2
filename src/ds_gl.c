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
#include <ds_gl.h>

G_DEFINE_QUARK(ds-gl-error-quark,
               ds_gl_error);

static GEnumClass*
ds_gl_error_get_class()
{
  static
  GEnumClass* klass = NULL;
  if G_UNLIKELY(klass == NULL)
  {
    GType g_type =
    ds_gl_error_get_type();

    klass =
    g_type_class_ref(g_type);
  }
return klass;
}

GError*
ds_gl_get_error()
{
  GEnumValue* value =
  g_enum_get_value
  (ds_gl_error_get_class(),
   glGetError());

  g_assert(value != NULL);

  return
  g_error_new
  (DS_GL_ERROR,
   value->value,
   "glGetError(): %s\r\n",
   value->value_nick);
}
