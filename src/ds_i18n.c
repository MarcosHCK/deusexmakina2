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
#include <ds_font.h>
#include <ds_macros.h>
#include <glib/gi18n.h>

G_GNUC_INTERNAL
gboolean
_ds_libi18n_get_charset(gchar** pcharset, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;

/*
 *
 *
 */

  static const gchar cs_symbols[] = " 0123456789!@#$%^&*()_/*-+`~:;\"\'<>,.?[]{}\\|";
  static const gchar cs_letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

  *pcharset = g_strjoin("", cs_symbols, cs_letters, NULL);

_error_:
return success;
}
