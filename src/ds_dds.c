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
#include <ds_dds.h>
#include <ds_gl.h>
#include <ds_macros.h>

G_DEFINE_QUARK(ds-dss-error-quark,
               ds_dds_error);

typedef struct _DDSFile         DDSFile;
typedef struct _DDSHeader       DDSHeader;
typedef struct _DDSPixelFormat  DDSPixelFormat;

#define _g_bytes_unref0(var) ((var == NULL) ? NULL : (var = (g_bytes_unref (var), NULL)))

/*
 * Structures
 *
 */

#pragma pack(push, 1)
#define _PACKED __attribute__((packed, aligned(1)))

struct _DDSPixelFormat
{
  guint32 size;
  guint32 flags;
  gchar fourcc[4];
  guint32 rgb_bits;
  guint32 red_mask;
  guint32 green_mask;
  guint32 blue_mask;
  guint32 alpha_mask;
} _PACKED;

struct _DDSHeader
{
  guint32 size;
  guint32 flags;
  guint32 height;
  guint32 width;
  guint32 linearsz;
  guint32 depth;
  guint32 n_mipmaps;
  guint32 reserved_1[11];
  DDSPixelFormat format;
  guint32 caps;
  guint32 caps2;
  guint32 caps3;
  guint32 caps4;
  guint32 reserved_2;
} _PACKED;

struct _DDSFile
{
  gchar magic[4];
  DDSHeader header;
} _PACKED;

#pragma pack(pop)

/* sanity checks */
G_STATIC_ASSERT(sizeof(DDSHeader) == (124));
G_STATIC_ASSERT(sizeof(DDSPixelFormat) == 32);
G_STATIC_ASSERT(sizeof(guint) >= sizeof(GLuint));

/*
 * Methods
 *
 */

gboolean
ds_dds_load_image(GFile          *file,
                  guint           gltype,
                  GCancellable   *cancellable,
                  GError        **error)
{
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  GBytes* pixels = NULL;

/*
 * Load file
 *
 */

  pixels =
  g_file_load_bytes(file, cancellable, NULL, &tmp_err);
  if G_UNLIKELY(tmp_err != NULL)
  {
    g_propagate_error(error, tmp_err);
    goto_error();
  }

  gsize pixelsz = 0;
  DDSFile* dds = (DDSFile*)
  g_bytes_get_data(pixels, &pixelsz);

  DDSHeader* header = &(dds->header);
  DDSPixelFormat* fmt_ = &(header->format);

/*
 * Check dds
 *
 */

  if G_UNLIKELY
    (memcmp(dds->magic, "DDS", 3) != 0
     || header->size != sizeof(DDSHeader))
  {
    g_set_error_literal
    (error,
     DS_DDS_ERROR,
     DS_DDS_ERROR_INVALID_HEADER,
     "Invalid header\r\n");
    goto_error();
  }

  if G_UNLIKELY
    (memcmp(fmt_->fourcc, "DXT", 3) != 0)
  {
    g_set_error_literal
    (error,
     DS_DDS_ERROR,
     DS_DDS_ERROR_INVALID_FOURCC,
     "Invalid four character code\r\n");
    goto_error();
  }

/*
 * Translate values
 * to GL ones
 *
 */

  GLuint width = header->width;
  GLuint height = header->height;
  guint n_mipmaps = header->n_mipmaps;
  GLenum fmt = GL_NONE;

  switch(fmt_->fourcc[3])
  {
  case '1':
    fmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    break;
  case '3':
    fmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    break;
  case '5':
    fmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    break;
  default:
    g_set_error
    (error,
     DS_DDS_ERROR,
     DS_DDS_ERROR_UNSUPPORTED_FORMAT,
     "Unsupported format '%.*s'\r\n",
     G_N_ELEMENTS(fmt_->fourcc),
     fmt_->fourcc);
    goto_error();
    break;
  }

/*
 * Load texture
 *
 */

  guint blocksz = (fmt == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
  gpointer pixels_ = (gpointer) (dds + 1);
  goffset offset = 0;
  guint i;

  for(i = 0;
      i < n_mipmaps
      && (width || height);
      i++)
  {
    gsize size = ((width + 3) / 4) * ((height + 3) / 4) * blocksz;

    __gl_try_catch(
      glCompressedTexImage2D((GLuint) gltype, i, fmt, width, height, 0, size, pixels_ + offset);
    ,
      g_propagate_error(error, glerror);
      goto_error();
    );

    offset += size;
    height /= 2;
    width /= 2;
  }

_error_:
  _g_bytes_unref0(pixels);
return success;
}
