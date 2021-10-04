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

[CCode (cheader_filename = "ds_dds_fmt.h")]
namespace DDS
{
  public enum Format
  {
    NONE,
    DXT1,
    DXT3,
    DXT5,
  }

  [Compact]
  public struct FourCC
  {
    public char code[4];
  }

  [Compact]
  public struct PixelFormat
  {
    public bool fourcc_is_valid();

    public uint32 size;
    public uint32 flags;
    public DDS.FourCC fourcc;
    public uint32 rgb_bits;
    public uint32 red_mask;
    public uint32 green_mask;
    public uint32 blue_mask;
    public uint32 alpha_mask;
  }

  [Compact]
  public struct Header
  {
    public bool size_matches();

    public uint32 size;
    public uint32 flags;
    public uint32 height;
    public uint32 width;
    public uint32 linearsz;
    public uint32 depth;
    public uint32 n_mipmaps;
    public DDS.PixelFormat format;
    public uint32 caps;
    public uint32 caps2;
    public uint32 caps3;
    public uint32 caps4;
  }

  [Compact]
  public struct File
  {
    public bool magic_is_valid();

    public DDS.FourCC magic;
    public DDS.Header header;
  }
}
