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
  const string AUTOSAVE = "autosave";

  public class Saves : GLib.Object, GLib.Initable
  {
    public string name {get; construct;}
    private GLib.File savesdir = null;
    private GLib.File file = null;

    public bool init(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      var time = new GLib.DateTime.now_local();
      string fullname = this._name + time.format("-%F-%T");
      this.file = savesdir.get_child(fullname);
    return true;
    }

    public GLib.OutputStream save(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      return file.create(GLib.FileCreateFlags.PRIVATE, cancellable) as GLib.OutputStream;
    }

    public GLib.InputStream load(GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      return file.read(cancellable) as GLib.InputStream;
    }

    public Saves(string name, GLib.File savesdir, GLib.Cancellable? cancellable = null) throws GLib.Error
    {
      GLib.Object(name: name);
      this.savesdir = savesdir;
      this.init(cancellable);
    }
  }
}
