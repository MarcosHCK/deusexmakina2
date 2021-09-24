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
#include <unicode/localebuilder.h>
#include <unicode/locid.h>
#include <string>
using namespace icu;

G_GNUC_INTERNAL
gboolean
_ds_icu_get_charset(gchar** pcharset, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  const gchar* const* langs = NULL;
  guint i;
  UErrorCode ecode = U_ZERO_ERROR;
  LocaleBuilder builder;
  Locale locale_;

/*
 * Inspect locales
 *
 */

  langs =
  g_get_language_names();

/*
 * Builder
 *
 */

  builder = LocaleBuilder();

  for(i = 0, success = FALSE;
      langs[i] != NULL;
      i++)
  {
    builder.clear();
    if(!g_strcmp0(langs[i], "C") == 1)
      builder.setLocale(langs[i]);
    else
      builder.setLocale("en");

    locale_ = builder.build(ecode);
    if G_UNLIKELY
      (U_SUCCESS(ecode) == TRUE)
    {
      success = TRUE;
      break;
    }
  }

  if G_UNLIKELY(success == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_LIBICU,
     "LocalBuilder::builder(): failed!\r\n");
    goto_error();
  }

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
