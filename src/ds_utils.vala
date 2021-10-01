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
  errordomain PushError
  {
    FAILED,
    UNKNOWN_TRANSLATION,
    UNEXPECTED_TYPE,
  }

  internal bool pushvalue(Lua.LuaVM L, GLib.Value value) throws GLib.Error
  {
    var gtype = value.type();
    switch(gtype)
    {
    case GLib.Type.CHAR:
      L.push_number((double) value.get_schar());
      break;
    case GLib.Type.UCHAR:
      L.push_number((double) value.get_uchar());
      break;
    case GLib.Type.BOOLEAN:
      L.push_boolean((int) value.get_boolean());
      break;
    case GLib.Type.INT:
      L.push_number((double) value.get_int());
      break;
    case GLib.Type.UINT:
      L.push_number((double) value.get_uint());
      break;
    case GLib.Type.LONG:
      L.push_number((double) value.get_long());
      break;
    case GLib.Type.ULONG:
      L.push_number((double) value.get_ulong());
      break;
    case GLib.Type.INT64:
      L.push_number((double) value.get_int64());
      break;
    case GLib.Type.UINT64:
      L.push_number((double) value.get_uint64());
      break;
    case GLib.Type.FLOAT:
      L.push_number((double) value.get_float());
      break;
    case GLib.Type.DOUBLE:
      L.push_number((double) value.get_double());
      break;
    case GLib.Type.STRING:
      L.push_string(value.get_string());
      break;
    case GLib.Type.POINTER:
      L.push_lightuserdata(value.get_pointer());
      break;
    default:
      if(gtype.is_a(GLib.Type.ENUM))
      {
        L.push_number((double) value.get_enum());
      } else
      if(gtype.is_a(GLib.Type.FLAGS))
      {
        L.push_number((double) value.get_flags());
      } else
      if(gtype.is_a(GLib.Type.BOXED))
      {
        luaD.pushboxed(L, gtype, value.get_boxed());
      } else
      if(gtype.is_a(GLib.Type.OBJECT))
      {
        luaD.pushobject(L, value.get_object());
      } else
      {
        throw new PushError.UNKNOWN_TRANSLATION
        ("unknown translation to Lua values for type '%s'\r\n",
         gtype.name());
      }
      break;
    }
  return true;
  }

  static bool checktype(GLib.Type expected, GLib.Type got) throws GLib.Error
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

  internal bool tovalue(Lua.LuaVM L, int idx, out GLib.Value value, GLib.Type g_type) throws GLib.Error
  {
    switch(L.type(idx))
    {
    case Lua.Type.NONE:
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
      if(luaD.isobject(L, idx) == true)
      {
        var obj = luaD.toobject(L, idx);
        value = GLib.Value(GLib.Type.OBJECT);
        value.set_object(obj);
      } else
      if(luaD.isboxed(L, idx) == true)
      {
        GLib.Type btype;
        var ptr = luaD.toboxed(L, idx, out btype);
        value = GLib.Value(btype);
        value.set_boxed(ptr);
      } else
      {
        throw new PushError.UNKNOWN_TRANSLATION
        ("unknown type in '%s' translation\r\n",
         L.type_name
         (L.type
          (idx)));
      }
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
