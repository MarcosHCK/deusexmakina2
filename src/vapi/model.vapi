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
  [CCode (cheader_filename = "ds_model.h")]
  public abstract class Model : GLib.Object, GLib.Initable
  {
    public GLib.File source {construct;}
    public string name {construct;}

    public uint vbo;
    public uint ibo;

    [Compact]
    public struct Vertex
    {
      public float position[3];
      public float normal[3];
      public float uvw[3];
      public float tangent[3];
      public float bitangent[3];
    }

    [Compact]
    public struct Index : uint
    {
    }

    [CCode (ref_function = "ds_model_texture_ref", unref_function = "ds_model_texture_unref")]
    [Compact]
    public class Texture
    {
      Texture() throws GLib.Error;
    }
  }
}
