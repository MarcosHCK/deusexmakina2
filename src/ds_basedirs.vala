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

namespace _Ds
{
  errordomain UtilsError
  {
    FAILED,
    BASE_DATA_DIR,
    BASE_CACHE_DIR,
  }

  const int F_DIRECTORY_MODE = 0x1c0 /* chmod 700 */;
  const string F_DIRECTORY_SEP = "/";
  const string PACKAGE_NAME = Config.PACKAGE_NAME;

  static bool check_dir(string path, GLib.Cancellable? cancellable = null) throws GLib.Error
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

  namespace base_dirs
  {
    internal GLib.File child(string name, GLib.File basedatadir, GLib.Cancellable? cancellable = null) throws GLib.Error
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

  namespace base_data_dir
  {
    internal GLib.File pick(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var paths = new GenericArray<string>();
      unowned string env = null;

      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string token = string.join("", ".", PACKAGE_NAME, null);
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, token, null));
      }

      if((env = GLib.Environment.get_variable("APPDATA")) != null)
      {
        string token = string.join("", ".", PACKAGE_NAME, null);
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, token, null));
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, PACKAGE_NAME, null));
      }

      if((env = GLib.Environment.get_variable("XDG_CONFIG_HOME")) != null)
      {
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, PACKAGE_NAME, null));
      } else
      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string path_ = GLib.Path.build_path(F_DIRECTORY_SEP, env, ".config/", null);
        if(GLib.FileUtils.test(path_, GLib.FileTest.IS_DIR))
          paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, ".config", PACKAGE_NAME, null));
      }

      if((env = GLib.Environment.get_variable("XDG_DATA_HOME")) != null)
      {
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, PACKAGE_NAME, null));
      } else
      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string path_ = GLib.Path.build_path(F_DIRECTORY_SEP, env, ".local/share/", null);
        if(GLib.FileUtils.test(path_, GLib.FileTest.IS_DIR))
          paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, ".local/share/", PACKAGE_NAME, null));
      }

      if(unlikely(paths.length == 0))
      {
        throw new _Ds.UtilsError.BASE_DATA_DIR("Could not find a suitable configuration path\r\n");
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
#if DEVELOPER == 1
        print("basedata: %s\r\n", baseDir);
#endif // DEVELOPER

      check_dir(baseDir, cancellable);
    return GLib.File.new_for_path(baseDir);
    }
  }

  namespace base_cache_dir
  {
    internal GLib.File pick(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var paths = new GenericArray<string>();
      unowned string env = null;

      var basedata = _Ds.base_data_dir.pick(cancellable);
      paths.add(basedata.get_child("cache").peek_path());

      if((env = GLib.Environment.get_variable("XDG_CACHE_HOME")) != null)
      {
        paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, PACKAGE_NAME));
      } else
      if((env = GLib.Environment.get_variable("HOME")) != null)
      {
        string path_ = GLib.Path.build_path(F_DIRECTORY_SEP, env, ".cache");
        if(GLib.FileUtils.test(path_, GLib.FileTest.IS_DIR))
          paths.add(GLib.Path.build_path(F_DIRECTORY_SEP, env, ".cache", PACKAGE_NAME));
      }

      if(unlikely(paths.length == 0))
      {
        throw new _Ds.UtilsError.BASE_CACHE_DIR("Could not find a suitable cache path\r\n");
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
        warning("Using legacy cache path '%s'\r\n" +
                "Please move this to '%s'\r\n",
                baseDir, preferred);
#if DEVELOPER == 1
        print("basecache: %s\r\n", baseDir);
#endif // DEVELOPER

      check_dir(baseDir, cancellable);
    return GLib.File.new_for_path(baseDir);
    }
  }
}
