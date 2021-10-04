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

namespace Ds
{
  [CCode (cheader_filename = "ds_gl.h")]
  namespace GL
  {
    public errordomain Error
    {
      FAILED,
      NO_ERROR,
      INVALID_ENUM,
      INVALID_VALUE,
      INVALID_OPERATION,
      INVALID_FRAMEBUFFER,
      OUT_OF_MEMORY,
      STACK_UNDERFLOW,
      STACK_OVERFLOW;
      public static GLib.Quark quark();
    }

    public static bool has_error();
    public static GLib.Error get_error();

    public struct @enum : uint
    {
    }

    public enum TextureType
    {
      1D,
      1D_ARRAY,
      2D,
      2D_ARRAY,
      3D,
      CUBEMAP,
      CUBEMAP_POSITIVE_X,
      CUBEMAP_NEGATIVE_X,
      CUBEMAP_POSITIVE_Y,
      CUBEMAP_NEGATIVE_Y,
      CUBEMAP_POSITIVE_Z,
      CUBEMAP_NEGATIVE_Z,
    }

    [CCode (cname = "glCompressedTexImage2D")]
    public static void CompressedTexImage2Dv(Ds.GL.TextureType type, uint level, GL.@enum internal_format, uint width, uint height, uint border, [CCode (array_length_cname = "n_bytes", array_length_pos = 2.5, array_length_type = "guint")] uint8[] bytes);
    [CCode (cname = "glCompressedTexImage2D")]
    public static void CompressedTexImage2D(Ds.GL.TextureType type, uint level, GL.@enum internal_format, uint width, uint height, uint border, size_t n_bytes, uint8* bytes);
    [CCode (cname = "glCompressedTexImage3D")]
    public static void CompressedTexImage3Dv(Ds.GL.TextureType type, uint level, GL.@enum internal_format, uint width, uint height, uint depth, uint border, [CCode (array_length_cname = "n_bytes", array_length_pos = 2.5, array_length_type = "guint")] uint8[] bytes);
    [CCode (cname = "glCompressedTexImage3D")]
    public static void CompressedTexImage3D(Ds.GL.TextureType type, uint level, GL.@enum internal_format, uint width, uint height, uint depth, uint border, size_t n_bytes, uint8* bytes);
    [CCode (cname = "glCompressedTexSubImage2D")]
    public static void CompressedTexSubImage2Dv(Ds.GL.TextureType type, uint level, uint xoffset, uint yoffset, uint width, uint height, GL.@enum format, [CCode (array_length_cname = "n_bytes", array_length_pos = 2.5, array_length_type = "guint")] uint8[] bytes);
    [CCode (cname = "glCompressedTexSubImage2D")]
    public static void CompressedTexSubImage2D(Ds.GL.TextureType type, uint level, uint xoffset, uint yoffset, uint width, uint height, GL.@enum format, size_t n_bytes, uint8* bytes);
    [CCode (cname = "glCompressedTexSubImage3D")]
    public static void CompressedTexSubImage3Dv(Ds.GL.TextureType type, uint level, uint xoffset, uint yoffset, uint zoffset, uint width, uint height, uint depth, GL.@enum format, [CCode (array_length_cname = "n_bytes", array_length_pos = 2.5, array_length_type = "guint")] uint8[] bytes);
    [CCode (cname = "glCompressedTexSubImage3D")]
    public static void CompressedTexSubImage3D(Ds.GL.TextureType type, uint level, uint xoffset, uint yoffset, uint zoffset, uint width, uint height, uint depth, GL.@enum format, size_t n_bytes, uint8* bytes);
  }
}
