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
#ifndef __DS_DDS_FMT_INCLUDED__
#define __DS_DDS_FMT_INCLUDED__ 1
#include <GL/glew.h>
#include <glib-object.h>

typedef struct _DDSFile         DDSFile;
typedef struct _DDSHeader       DDSHeader;
typedef struct _DDSPixelFormat  DDSPixelFormat;
typedef struct _DDSFourCC       DDSFourCC;

typedef enum
{
  DDS_FORMAT_NONE = GL_NONE,                          /*<nick=GL_NONE>*/
  DDS_FORMAT_DXT1 = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, /*<nick=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT>*/
  DDS_FORMAT_DXT3 = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, /*<nick=GL_COMPRESSED_RGBA_S3TC_DXT3_EXT>*/
  DDS_FORMAT_DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, /*<nick=GL_COMPRESSED_RGBA_S3TC_DXT5_EXT>*/
} DDSFormat;

#pragma pack(push, 1)
#define _PACKED __attribute__((packed, aligned(1)))

struct _DDSFourCC
{
  union
  {
    gchar code[4];
    struct
    {
      gchar magic[3];
      gchar bit;
    };
  };
} _PACKED;

struct _DDSPixelFormat
{
  guint32 size;
  guint32 flags;
  DDSFourCC fourcc;
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
  DDSFourCC magic;
  DDSHeader header;
} _PACKED;

#pragma pack(pop)

/* sanity checks */
G_STATIC_ASSERT(sizeof(DDSHeader) == (124));
G_STATIC_ASSERT(sizeof(DDSPixelFormat) == 32);

#if __cplusplus
extern "C" {
#endif // __cplusplus

GType
dds_format_get_type();

static inline gboolean
dds_file_magic_is_valid(DDSFile* file)
{
  return memcmp(file->magic.code, "DDS", 3) == 0;
}

static inline gboolean
dds_header_size_matches(DDSHeader* header)
{
  return header->size == sizeof(DDSHeader);
}

static inline gboolean
dds_pixel_format_fourcc_is_valid(DDSPixelFormat* fmt)
{
  return memcmp(fmt->fourcc.code, "DXT", 3) == 0;
}

#if __cplusplus
}
#endif // __cplusplus

#endif // __DS_DDS_FMT_INCLUDED__
