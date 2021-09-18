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
  errordomain UtilsError
  {
    FAILED,
    BASE_DATA_DIR,
  }
}

namespace _Ds
{
  const int F_DIRECTORY_MODE = 0x1c0 /* chmod 700 */;
  const string F_DIRECTORY_SEP = "/";
  const string CONFIG_ROOT = Config.PACKAGE_NAME;

  private bool check_dir(string path, GLib.Cancellable? cancellable = null) throws GLib.Error
  {
    bool  exists = GLib.FileUtils.test(path, GLib.FileTest.EXISTS),
          isfolder = GLib.FileUtils.test(path, GLib.FileTest.IS_DIR);

    if(unlikely
       ((exists == true)
        && (isfolder == false)))
      throw new GLib.IOError.NOT_DIRECTORY("Directory path '%s' exists, but is not a directory\r\n", path);
    else
    if(exists == false)
      if(GLib.DirUtils.create_with_parents(path, F_DIRECTORY_MODE) != 0)
        throw new GLib.IOError.NOT_SUPPORTED("Failed to create directory '%s'\r\n", path);
  return true;
  }

  namespace base_data_dir
  {
    public GLib.File pick(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var paths = new GenericArray<string>();
      unowned string env = null;

      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string token = string.join("", ".", CONFIG_ROOT, null);
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, token, null));
      }

      if((env = GLib.Environment.get_variable("APPDATA")) != null)
      {
        string token = string.join("", ".", CONFIG_ROOT, null);
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, token, null));
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, CONFIG_ROOT, null));
      }

      if((env = GLib.Environment.get_variable("XDG_CONFIG_HOME")) != null)
      {
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, CONFIG_ROOT, null));
      } else
      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string path_ = GLib.Path.build_path(F_DIRECTORY_SEP, env, ".config/", null);
        if(GLib.FileUtils.test(path_, GLib.FileTest.IS_DIR))
          paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, ".config", CONFIG_ROOT, null));
      }

      if((env = GLib.Environment.get_variable("XDG_DATA_HOME")) != null)
      {
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, CONFIG_ROOT, null));
      } else
      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string path_ = GLib.Path.build_path(F_DIRECTORY_SEP, env, ".local/share/", null);
        if(GLib.FileUtils.test(path_, GLib.FileTest.IS_DIR))
          paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, ".local/share/", CONFIG_ROOT, null));
      }

      if(unlikely(paths.length == 0))
      {
        throw new Ds.UtilsError.BASE_DATA_DIR("Could not find a suitable configuration path\r\n");
      }

      unowned string baseDir = null;
      unowned string preferred = paths[paths.length - 1];

      for(uint i = 0;i < paths.length;i++)
      if(GLib.FileUtils.test(paths[i], GLib.FileTest.EXISTS))
      {
        baseDir = paths[i];
        break;
      }

      if(baseDir == null)
        baseDir = preferred;

      if(unlikely(baseDir != preferred))
        warning("Using legacy configuration path '%s'\r\n" +
                "Please move this to '%s'\r\n",
                baseDir, preferred);
      if(unlikely(Config.DEBUG) == true)
        print("basedata: %s\r\n", baseDir);

      check_dir(baseDir, cancellable);
    return GLib.File.new_for_path(baseDir);
    }

    public GLib.File child(string name, GLib.File basedatadir, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var child = basedatadir.get_child(name);
      try {
        child.make_directory_with_parents(cancellable);
      }
      catch(GLib.Error e) {
        if(unlikely
           (e.matches
            (GLib.IOError.quark(),
             GLib.IOError.EXISTS) == false))
        {
          throw e;
        }
      }

      if(unlikely(Config.DEBUG) == true)
        print("%s: %s\r\n", name, child.peek_path());
    return child;
    }
  }

  errordomain PushError
  {
    FAILED,
    UNKNOWN_TRANSLATION,
    UNEXPECTED_TYPE,
  }

  private bool checktype(GLib.Type expected, GLib.Type got) throws GLib.Error
  {
    if(expected != GLib.Type.INVALID)
    {
      if(unlikely(got.is_a(expected) == false))
      {
        throw new PushError.UNEXPECTED_TYPE
        ("expected type '%s', got '%s'\r\n",
         expected.name(),
         got.name());
      }
    }
  return true;
  }

  public bool tovalue(Lua.LuaVM L, int idx, out GLib.Value value, GLib.Type g_type) throws GLib.Error
  {
    switch(L.type(idx))
    {
    case Lua.Type.NIL:
      if(g_type != GLib.Type.INVALID)
      {
        value = GLib.Value(g_type);
      }
      else
      {
        throw new PushError.UNKNOWN_TRANSLATION
        ("ambiguous translation rule for lua type 'nil'\r\n");
      }
      break;
    case Lua.Type.BOOLEAN:
      checktype(g_type, GLib.Type.BOOLEAN);
      value = GLib.Value(GLib.Type.BOOLEAN);
      value.set_boolean(L.to_boolean(idx));
      break;
    case Lua.Type.LIGHTUSERDATA:
      checktype(g_type, GLib.Type.POINTER);
      value = GLib.Value(GLib.Type.POINTER);
      value.set_pointer(L.to_userdata(idx));
      break;
    case Lua.Type.NUMBER:
      if(g_type == GLib.Type.INVALID)
      {
        value = GLib.Value(GLib.Type.DOUBLE);
        value.set_double((double) L.to_number(idx));
      }
      else
      switch(g_type)
      {
      case GLib.Type.FLOAT:
        value = GLib.Value(g_type);
        value.set_float((float) L.to_number(idx));
        break;
      case GLib.Type.DOUBLE:
        value = GLib.Value(g_type);
        value.set_double((double) L.to_number(idx));
        break;
      case GLib.Type.CHAR:
        value = GLib.Value(g_type);
        value.set_schar((int8) L.to_number(idx));
        break;
      case GLib.Type.INT:
        value = GLib.Value(g_type);
        value.set_int((int) L.to_number(idx));
        break;
      case GLib.Type.LONG:
        value = GLib.Value(g_type);
        value.set_long((long) L.to_number(idx));
        break;
      case GLib.Type.UCHAR:
        value = GLib.Value(g_type);
        value.set_uchar((uint8) L.to_number(idx));
        break;
      case GLib.Type.UINT:
        value = GLib.Value(g_type);
        value.set_uint((uint) L.to_number(idx));
        break;
      case GLib.Type.ULONG:
        value = GLib.Value(g_type);
        value.set_ulong((ulong) L.to_number(idx));
        break;
      case GLib.Type.INT64:
        value = GLib.Value(g_type);
        value.set_int64((int64) L.to_number(idx));
        break;
      case GLib.Type.UINT64:
        value = GLib.Value(g_type);
        value.set_uint64((uint64) L.to_number(idx));
        break;
      default:
        if(g_type.is_a(GLib.Type.ENUM) == true)
        {
          value = GLib.Value(g_type);
          value.set_enum((int) L.to_number(idx));
        } else
        if(g_type.is_a(GLib.Type.FLAGS) == true)
        {
          value = GLib.Value(g_type);
          value.set_flags((uint) L.to_number(idx));
        } else
        {
          throw new PushError.UNEXPECTED_TYPE
          ("expected type '%s'\r\n",
           g_type.name());
        }
        break;
      }
      break;
    case Lua.Type.STRING:
      checktype(g_type, GLib.Type.STRING);
      value = GLib.Value(GLib.Type.STRING);
      value.set_string(L.to_string(idx));
      break;
    case Lua.Type.FUNCTION:
      //checktype(g_type, GLib.Type.CLOSURE);
      //value = GLib.Value(GLib.Type.CLOSURE);
      //value.set_string(L.to_string(idx));
      assert_not_reached();
      break;
    case Lua.Type.USERDATA:
      assert_not_reached();
      break;
    default:
      throw new PushError.UNKNOWN_TRANSLATION
      ("unknown type in '%s' translation\r\n",
       L.type_name
       (L.type
        (idx)));
    }
  return true;
  }
}
