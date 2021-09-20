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
  [Compact]
  public class Closure : GLib.Closure
  {
  }
}

[CCode (cprefix = "luaD_", lower_case_cprefix = "luaD_", cheader_filename = "ds_luavala.h")]
namespace luaD
{
  public void pushboxed(Lua.LuaVM L, GLib.Type g_type, void* instance);
  public bool isboxed(Lua.LuaVM L, int idx);
  public void* toboxed(Lua.LuaVM L, int idx, out GLib.Type g_type);
  public void* checkboxed(Lua.LuaVM L, int idx, out GLib.Type g_type);
  public void pushclosure(Lua.LuaVM L, Ds.Closure closure);
  public bool isclosure(Lua.LuaVM L, int idx);
  public Ds.Closure toclosure(Lua.LuaVM L, int idx);
  public Ds.Closure checkclosure(Lua.LuaVM L, int idx);
  public void pushobject(Lua.LuaVM L, GLib.Object object);
  public bool isobject(Lua.LuaVM L, int idx);
  public GLib.Object toobject(Lua.LuaVM L, int idx);
  public GLib.Object checkobject(Lua.LuaVM L, int idx);
}
