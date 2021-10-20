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
  errordomain FolderProviderError
  {
    FAILED,
    INVALID,
  }

/*
 * Base class
 *
 */

  public abstract class FolderProvider : GLib.Object, GLib.Initable
  {
    protected GLib.File basedir = null;
    protected abstract GLib.File pick_basedir(GLib.Cancellable? cancellable = null) throws GLib.Error;

    public GLib.File child(string name, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      return Ds.base_dirs.child(name, this.basedir, cancellable);
    }

    public bool init(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      this.basedir = pick_basedir(cancellable);
    return true;
    }
  }

/*
 * Implementations
 *
 */

  public class DataProvider : Ds.FolderProvider
  {
    public override GLib.File pick_basedir(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      return Ds.base_data_dir.pick(cancellable);
    }

    static weak Ds.DataProvider __default__ = null;
    public static unowned Ds.DataProvider get_default()
    {
      return __default__;
    }

    public DataProvider(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      GLib.Object();
      this.init(cancellable);
      __default__ = this;
    }
    ~DataProvider()
    {
      __default__ = null;
    }
  }

  public class CacheProvider : Ds.FolderProvider
  {
    public override GLib.File pick_basedir(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      return Ds.base_cache_dir.pick(cancellable);
    }

    static weak Ds.CacheProvider __default__ = null;
    public static unowned Ds.CacheProvider get_default()
    {
      return __default__;
    }

    public CacheProvider(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      GLib.Object();
      this.init(cancellable);
      __default__ = this;
    }
    ~CacheProvider()
    {
      __default__ = null;
    }
  }
}
