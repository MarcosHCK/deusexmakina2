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
  public errordomain GirHubError
  {
    FAILED,
    UNKNOWN_TYPE,
    UNSUPPORTED_TYPE,
    UNKNOWN_TRANSLATION,
  }

  public class GirHub : GLib.Object
  {
    private GLib.HashTable<string,Ds.Closure> closures = null;
    private GLib.HashTable<string,GI.BaseInfo> itypes = null;

    static GLib.Module __main__ = null;
    private GLib.Callback? resolve_symbol(string name)
    {
      GLib.Callback callback = null;
      if(unlikely(__main__ == null))
        __main__ = GLib.Module.open(null, 0);

      void* symbol;
      if(__main__.symbol(name, out symbol))
        callback = (GLib.Callback) symbol;
      if(Config.DEBUG == true)
        print(@"symbol '$name'\r\n");
    return callback;
    }

    private GLib.Type translate_itype(GI.TypeInfo info)
    {
      switch(info.get_tag())
      {
      case GI.TypeTag.BOOLEAN:
        return GLib.Type.BOOLEAN;
      case GI.TypeTag.INT8:
        return GLib.Type.CHAR;
      case GI.TypeTag.UINT8:
        return GLib.Type.UCHAR;
      case GI.TypeTag.INT16:
        return GLib.Type.INT;
      case GI.TypeTag.UINT16:
        return GLib.Type.UINT;
      case GI.TypeTag.INT32:
        return GLib.Type.INT;
      case GI.TypeTag.UINT32:
        return GLib.Type.UINT;
      case GI.TypeTag.INT64:
        return GLib.Type.INT64;
      case GI.TypeTag.UINT64:
        return GLib.Type.UINT64;
      case GI.TypeTag.FLOAT:
        return GLib.Type.FLOAT;
      case GI.TypeTag.DOUBLE:
        return GLib.Type.DOUBLE;
      case GI.TypeTag.GTYPE:
        return Ds.Types.GTYPE;
      case GI.TypeTag.UTF8:
        return GLib.Type.STRING;
      case GI.TypeTag.FILENAME:
        return GLib.Type.STRING;
      case GI.TypeTag.ARRAY:
        {
          switch(info.get_array_type())
          {
          case GI.ArrayType.C:
            return GLib.Type.POINTER;
          case GI.ArrayType.ARRAY:
            return Ds.Types.ARRAY;
          case GI.ArrayType.PTR_ARRAY:
            return Ds.Types.PTR_ARRAY;
          case GI.ArrayType.BYTE_ARRAY:
            return Ds.Types.BYTE_ARRAY;
          }
        }
        break;
      case GI.TypeTag.INTERFACE:
        return GLib.Type.OBJECT;
      case GI.TypeTag.GLIST:
      case GI.TypeTag.GSLIST:
        return GLib.Type.POINTER;
      case GI.TypeTag.GHASH:
        return Ds.Types.HASH_TABLE;
      case GI.TypeTag.ERROR:
        return Ds.Types.ERROR;
      }
    return GLib.Type.INVALID;
    }

    private Ds.Closure? translate_closure(GLib.Type g_type, GI.FunctionInfo func) throws GLib.Error
    {
      GI.CallableInfo info = (GI.CallableInfo) func;
      Ds.Closure closure = null;

    /*
     * Prepare types
     *
     */

      bool method = info.is_method();
      bool throws_ = info.can_throw_gerror();
      int n_args = info.get_n_args();
      int n_values = n_args + ((throws_) ? 1 : 0);
      int i;

      GLib.Type[] values = new GLib.Type[n_values];
      GLib.Type   return_ = GLib.Type.NONE;
      if(throws_)
      {
        values[n_args] = Ds.Types.ERROR;
      }

    /*
     * Load types
     *
     */

      for(i = 0;
          i < n_args;
          i++)
      {
        var arg = info.get_arg(i);
        var itype = arg.get_type();
        var type = translate_itype(itype);

        if(unlikely(type == GLib.Type.INVALID))
          throw new GirHubError.UNKNOWN_TRANSLATION(@"Unkown GI.TypeTag on translation '$(itype.get_tag())'");
        else
          values[i] = type;
      }

      var itype = info.get_return_type();
      if(itype.get_tag() != GI.TypeTag.VOID)
      {
        var type = translate_itype(itype);
        if(unlikely(type == GLib.Type.INVALID))
          throw new GirHubError.UNKNOWN_TRANSLATION(@"Unkown GI.TypeTag on translation '$(itype.get_tag())'");
        else
          return_ = type;
      }

      var callback = resolve_symbol(func.get_symbol());
      if(unlikely(callback == null))
        return closure;

    /*
     * Create closure
     *
     */

      closure = new Ds.Closure((method) ? ClosureFlags.FLAGS_NONE : ClosureFlags.CONSTRUCTOR, callback, null, null, return_, n_values, values);
    return closure;
    }

    public Ds.Closure? get_method(GLib.Type g_type, string name) throws GLib.Error
    {
      var cname = @"$(g_type.name())::$name";

      Ds.Closure closure =
      closures.lookup(cname);
      if(unlikely(closure == null))
      {
        GI.BaseInfo base_ =
        itypes.lookup(g_type.name());
        if(unlikely(base_ == null))
        {
          var repo = GI.Repository.get_default();
          base_ = repo.find_by_gtype(g_type);
//throw new GirHubError.UNKNOWN_TYPE(@"Unknown type '$(g_type.name())'");
          if(likely(base_ != null))
            itypes.insert(g_type.name(), base_);
          else
          {
            foreach(var iface in g_type.interfaces())
            {
              closure =
              this.get_method(iface, name);
              if(unlikely(closure != null))
              {
                return closure;
              }
            }

            var parent = g_type.parent();
            if(parent != GLib.Type.INVALID && parent != GLib.Type.INTERFACE)
              closure = this.get_method(parent, name);
          }
        }

        GI.FunctionInfo func = null;

        switch(base_.get_type())
        {
        case GI.InfoType.OBJECT:
          func = ((GI.ObjectInfo) base_).find_method_using_interfaces(name, null);
          break;
        case GI.InfoType.INTERFACE:
          func = ((GI.InterfaceInfo) base_).find_method(name);
          break;
        default:
          throw new GirHubError.UNSUPPORTED_TYPE(@"Unsupported type '$(g_type.name())'");
          break;
        }

        if(unlikely(func != null))
        {
          closure =
          translate_closure(g_type, func);
          if(likely(closure != null))
          {
            closures.insert(cname, closure);
          }
        }
        else
        {
          var parent = g_type.parent();
          if(likely(parent != GLib.Type.INVALID && parent != GLib.Type.INTERFACE))
            closure = this.get_method(parent, name);
        }
      }
    return closure;
    }

    static weak Ds.GirHub __default__ = null;
    public static unowned Ds.GirHub? get_default()
    {
      return __default__;
    }

    public GirHub()
    {
      Object();
      closures = new GLib.HashTable<string,Ds.Closure>(GLib.str_hash, GLib.str_equal);
      itypes = new GLib.HashTable<string,GI.BaseInfo>(GLib.str_hash, GLib.str_equal);
      __default__ = this;
    }
    ~GirHub()
    {
      __default__ = null;
    }
  }
}

[CCode (cprefix = "_ds_", lower_case_cprefix = "_ds_", cheader_filename = "ds_luavala.h")]
namespace _Ds
{
  public bool luagir_init(Lua.LuaVM L) throws GLib.Error
  {
    var object_ = new Ds.GirHub();
    object_.@ref();

/*
 * Load repos
 *
 */

    var repo = GI.Repository.get_default();
    repo.require("GObject", @"$(GLib.Version.MAJOR).0", 0);
    repo.require("Gio", @"$(GLib.Version.MAJOR).0", 0);
  return true;
  }

  public void luagir_fini(Lua.LuaVM L)
  {
    (Ds.GirHub.get_default()).unref();
  }
}
