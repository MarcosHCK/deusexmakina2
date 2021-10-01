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

[CCode (cheader_filename = "ds_closure.h")]
namespace Ds
{
  [Flags]
  [CCode (cprefix = "DS_CLOSURE_")]
  public enum ClosureFlags
  {
    FLAGS_NONE,
    CONSTRUCTOR,
  }

  [CCode (cheader_filename = "glib-object.h", has_target = false, cname = "GClosureMarshal")]
  public delegate void ClosureMarshal (GLib.Closure closure, GLib.Value? return_value, [CCode (array_length_cname = "n_param_values", array_length_pos = 2.5, array_length_type = "guint")] GLib.Value[] param_values, void* invocation_hint, void* marshal_data);
  [CCode (cheader_filename = "glib-object.h", has_target = false, cname = "GVaClosureMarshal")]
  public delegate void VaClosureMarshal (GLib.Closure closure, GLib.Value? return_value, GLib.TypeInstance instance, va_list args, void* marshal_data, [CCode (array_length_cname = "n_params", array_length_pos = 5.5)] GLib.Type[] param_types);

  [Compact]
  [CCode (ref_function = "ds_closure_ref", unref_function = "ds_closure_unref", ref_sink_function = "ds_closure_ref_sink")]
  public class Closure : GLib.Closure
  {
    public Closure(ClosureFlags flags, GLib.Callback addr, Ds.ClosureMarshal? marshal, Ds.VaClosureMarshal? vmarshal, GLib.Type return_type, uint n_params, GLib.Type* @params);

    public GLib.Callback callback;
    public GLib.Callback vmarshal;
    public ClosureFlags flags;
    public GLib.Type return_type;
    public uint n_params;
    public GLib.Type* @params;
  }

  [CCode (cprefix = "G_TYPE_", lower_case_cprefix = "G_TYPE_")]
  namespace Types
  {
    public GLib.Type GTYPE;
    public GLib.Type ARRAY;
    public GLib.Type PTR_ARRAY;
    public GLib.Type BYTE_ARRAY;
    public GLib.Type HASH_TABLE;
    public GLib.Type ERROR;
  }
}
