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
  public errordomain DdsError
  {
    FAILED,
    INVALID_MAGIC,
    INVALID_HEADER,
    INVALID_FOURCC,
    UNSUPPORTED_FORMAT,
  }

  public class Dds : GLib.Object, GLib.Initable
  {
    public GLib.File source {construct;}
    private GLib.Bytes contents;
    private DDS.File dds;

    public uint width
    {
      get
      {
        return dds.header.width;
      }
    }

    public uint height
    {
      get
      {
        return dds.header.height;
      }
    }

    public uint n_mipmap
    {
      get
      {
        return dds.header.n_mipmaps;
      }
    }

    public uint gl_internal_format
    {
      get
      {
        switch(dds.header.format.fourcc.code[3])
        {
        case '1':
          return DDS.Format.DXT1;
        case '3':
          return DDS.Format.DXT3;
        case '5':
          return DDS.Format.DXT5;
        }
      return DDS.Format.NONE;
      }
    }

/*
 * Real code
 *
 */

    public bool init(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var stream = (GLib.InputStream) _source.read(cancellable);

      unowned var magic = (uint8[]) &(dds.magic);
      unowned var header = (uint8[]) &(dds.header);

      stream.read(magic, cancellable);
      if(likely(dds.magic_is_valid() == true))
      {
        stream.read(header, cancellable);

        if(unlikely(dds.header.size_matches() == false))
          throw new DdsError.INVALID_HEADER("Invalid header size");
        if(unlikely(dds.header.format.fourcc_is_valid() == false))
          throw new DdsError.INVALID_FOURCC("Invalid fourcc magic present on file");

        var output = new GLib.MemoryOutputStream.resizable();
        output.splice(stream, GLib.OutputStreamSpliceFlags.CLOSE_TARGET, cancellable);
        this.contents = ((GLib.MemoryOutputStream) output).steal_as_bytes();
      }
      else
      {
        throw new DdsError.INVALID_MAGIC("Invalid magic present on file");
      }

      stream.close(cancellable);
    return true;
    }

    private uint gl_internal_format_p() throws GLib.Error
    {
      uint format = this.gl_internal_format;
      if(unlikely(format == DDS.Format.NONE))
      {
        throw new DdsError.UNSUPPORTED_FORMAT("Unsupported format");
      }
    return format;
    }

    public bool load_into_texture_2d(bool create, uint gl_type) throws GLib.Error
    {
      unowned var header = dds.header;
      unowned var fmt_ = header.format;

      uint width = this.width;
      uint height = this.height;
      uint n_mipmap = this.n_mipmap;
      uint gl_format = this.gl_internal_format_p();
      uint blocksz = (gl_format == DDS.Format.DXT1) ? 8 : 16;
      uint8* pixels = contents.get_data();
      size_t offset = 0;

      for
      (uint i = 0;
            i < n_mipmap
            && (width > 0 || height > 0);
            i++)
      {
        size_t size = ((width + 3) / 4) * ((height + 3) / 4) * blocksz;

        if(create)
          GL.CompressedTexImage2D((GL.TextureType) gl_type, i, gl_format, width, height, 0, size, pixels + offset);
        else
          GL.CompressedTexSubImage2D((GL.TextureType) gl_type, i, 0, 0, width, height, gl_format, size, pixels + offset);
        if(unlikely(GL.has_error()))
          throw GL.get_error();

        offset += size;
        height >>= 1;   /* height / 2 */
        width >>= 1;    /* width / 2  */
      }
    return true;
    }

    public bool load_into_texture_3d(bool create, uint gl_type, uint depth) throws GLib.Error
    {
      unowned var header = dds.header;
      unowned var fmt_ = header.format;

      uint width = this.width;
      uint height = this.height;
      uint n_mipmap = this.n_mipmap;
      uint gl_format = this.gl_internal_format_p();
      uint blocksz = (gl_format == DDS.Format.DXT1) ? 8 : 16;
      uint8* pixels = contents.get_data();
      size_t offset = 0;

      for
      (uint i = 0;
            i < n_mipmap
            && (width > 0 || height > 0);
            i++)
      {
        size_t size = ((width + 3) / 4) * ((height + 3) / 4) * blocksz;

        if(create)
          GL.CompressedTexImage3D((GL.TextureType) gl_type, i, gl_format, width, height, depth, 0, size, pixels + offset);
        else
          GL.CompressedTexSubImage3D((GL.TextureType) gl_type, i, 0, 0, depth, width, height, 1, gl_format, size, pixels + offset);
        if(unlikely(GL.has_error()))
          throw GL.get_error();

        offset += size;
        height >>= 1;   /* height / 2 */
        width >>= 1;    /* width / 2  */
      }
    return true;
    }

/*
 * Construction
 *
 */

    public Dds(GLib.File source, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      Object(source: source);
      this.init(cancellable);
    }

/*
 * Static functions
 *
 */

    public static bool load_image(GLib.File source, uint gl_type, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var dds = new Ds.Dds(source, cancellable);
      switch(gl_type)
      {
      case GL.TextureType.2D:
      case GL.TextureType.CUBEMAP_POSITIVE_X:
      case GL.TextureType.CUBEMAP_NEGATIVE_X:
      case GL.TextureType.CUBEMAP_POSITIVE_Y:
      case GL.TextureType.CUBEMAP_NEGATIVE_Y:
      case GL.TextureType.CUBEMAP_POSITIVE_Z:
      case GL.TextureType.CUBEMAP_NEGATIVE_Z:
        dds.load_into_texture_2d(true, gl_type);
        break;
      default:
        throw new DdsError.FAILED("Invalid value for GL texture type");
        break;
      }
    return true;
    }
  }
}
