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
#include <ds_application_private.h>
#include <ds_font.h>
#include <ds_macros.h>
#include <webp/encode.h>

typedef struct _DsWebPData DsWebPData;

struct _DsWebPData
{
  union
  {
    GOutputStream* output;
    GInputStream* input;
  };

  GCancellable* cancellable;
  GError* error;
};

static int
webp_writer(const uint8_t* bytes, size_t bytesz, const WebPPicture* pic)
{
  DsWebPData* data =
  (DsWebPData*) pic->custom_ptr;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  gsize wrote = 0;

  success =
  g_output_stream_write_all
  (data->output,
   bytes,
   bytesz,
   &wrote,
   data->cancellable,
   &tmp_err);

  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(&(data->error), tmp_err);
    goto_error();
  }

_error_:
return success;
}

static GFile*
get_charmap_cache_file(guint charset_hash, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  DsApplication* app = NULL;
  GFile* return_ = NULL;
  GFile* basedir = NULL;
  GFile* shader = NULL;

/*
 * Generate name
 *
 */

  app = (DsApplication*)
  g_application_get_default();
  g_assert(DS_IS_APPLICATION(app));

  gchar buf[64];
  g_snprintf(buf, sizeof(buf), "%x.shader", charset_hash);

  basedir =
  _ds_base_dirs_child("shaders", app->glcachedir, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  shader =
  g_file_get_child(basedir, buf);

/*
 * Finalize
 *
 */

  return_ = g_steal_pointer(&shader);

_error_:
  g_clear_object(&basedir);
  g_clear_object(&shader);
return return_;
}

G_GNUC_INTERNAL
gboolean
_ds_font_cache_try_load(guint charset_hash, GBytes** pbytes, gint* pw, gint* ph, GCancellable* cancellable, GError** error)
{
  g_set_error_literal
  (error,
   DS_FONT_ERROR,
   DS_FONT_ERROR_INVALID_CACHE,
   "Invalid cache\r\n");
return FALSE;
}


G_GNUC_INTERNAL
gboolean
_ds_font_cache_try_save(guint charset_hash, GBytes* bytes, gint w, gint h, GCancellable* cancellable, GError** error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GOutputStream* output = NULL;
  GFile* outfile = NULL;
  WebPConfig config = {0};
  WebPPicture pic = {0};
  gint return_ = 0;

/*
 * Get cache file
 *
 */

  outfile =
  get_charmap_cache_file(charset_hash, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Open file
 *
 */

  output = (GOutputStream*)
  g_file_replace(outfile, NULL, TRUE, G_FILE_CREATE_PRIVATE, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

/*
 * Fill default preset
 *
 */

  return_ =
  WebPConfigPreset(&config, WEBP_PRESET_TEXT, 90.f);
  if G_UNLIKELY(return_ == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_LIBWEBP,
     "WebPConfigPreset(): failed!\r\n");
    goto_error();
  }

/*
 * Fine-tune configuration
 *
 */

  config.lossless = TRUE;
  config.sns_strength = 90;
  config.filter_sharpness = 6;
  config.alpha_quality = 90;

  return_ =
  WebPValidateConfig(&config);
  if G_UNLIKELY(return_ == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_LIBWEBP,
     "WebPValidateConfig(): failed!\r\n");
    goto_error();
  }

/*
 * Prepate structures
 *
 */

  return_ =
  WebPPictureInit(&pic);
  if G_UNLIKELY(return_ == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_LIBWEBP,
     "WebPPictureInit(): failed!\r\n");
    goto_error();
  }

  pic.width = w;
  pic.height = h;

  return_ =
  WebPPictureAlloc(&pic);
  if G_UNLIKELY(return_ == FALSE)
  {
    g_set_error_literal
    (error,
     DS_FONT_ERROR,
     DS_FONT_ERROR_LIBWEBP,
     "WebPPictureAlloc(): failed!\r\n");
    goto_error();
  }

  DsWebPData data;
  data.output = output;
  data.cancellable = cancellable;
  data.error = NULL;

  pic.writer = webp_writer;
  pic.custom_ptr = &data;

  return_ =
  WebPEncode(&config, &pic);
  if G_UNLIKELY(return_ == FALSE)
  {
    if(data.error != NULL)
      g_propagate_error(error, data.error);
    else
      g_set_error
      (error,
       DS_FONT_ERROR,
       DS_FONT_ERROR_LIBWEBP,
       "WebPEncode(): failed!: %i\r\n",
       pic.error_code);
    goto_error();
  }

  success =
  g_output_stream_close(output, cancellable, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

_error_:
  _g_object_unref0(output);
  _g_object_unref0(outfile);
  WebPPictureFree(&pic);
return success;
}
