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

[CCode (cprefix = "luaD", lower_case_cprefix = "luad_")]
namespace luaD
{
  public class GirHub : GLib.Object
  {
    static weak luaD.GirHub __default__ = null;
    public static unowned luaD.GirHub? get_default()
    {
      return __default__;
    }

    public GirHub()
    {
      GLib.Object();
      __default__ = this;
    }
    ~GirHub()
    {
      __default__ = null;
    }
  }
}

[CCode (cprefix = "_luaD", lower_case_cprefix = "_luaD_", cheader_filename = "luad_girhub.h")]
namespace _luaD
{
  public bool girhub_init(Lua.LuaVM L) throws GLib.Error
  {
/*
 * Load repos
 *
 */

    var repo = GI.Repository.get_default();
    repo.require("GLib", @"$(GLib.Version.MAJOR).0", 0);
    repo.require("GObject", @"$(GLib.Version.MAJOR).0", 0);
    repo.require("Gio", @"$(GLib.Version.MAJOR).0", 0);

    if(Config.DEBUG)
      repo.require_private(Config.ABSTOPBUILDDIR + "/gir/", "Ds", @"$(Config.PACKAGE_VERSION_MAYOR).$(Config.PACKAGE_VERSION_MINOR)", 0);
    else
      repo.require_private(Config.GIRDIR, "Ds", @"$(Config.PACKAGE_VERSION_MAYOR).$(Config.PACKAGE_VERSION_MINOR)", 0);

/*
 * Create hub
 *
 */

    var object_ = new luaD.GirHub();
    object_.@ref();
  return true;
  }

  public void girhub_fini(Lua.LuaVM L)
  {
    (luaD.GirHub.get_default()).unref();
  }
}
