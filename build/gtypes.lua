--[[
-- Copyright 2020-2021 MarcosHCK
-- This file is part of deusexmakina2.
--
-- deusexmakina2 is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- deusexmakina2 is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with deusexmakina2.  If not, see <http://www.gnu.org/licenses/>.
--]]

local peek_prefix = 'g_marshal_value_peek_';
local set_prefix = 'g_value_set_';
local take_prefix = 'g_value_take_';

local gtypes;
gtypes = {
--[[
--
-- Default types
--
--]]

  VOID = {
    ctype = function()
      return 'void';
    end,
    artype = function()
      error('Type \'VOID\' can\'t be copied from a vararg list');
    end,
    peek = function()
      error('Type \'VOID\' is not a valid argument type');
    end,
    set = function()
      error('Type \'VOID\' has not a setter function');
    end,
  },
  NONE = {
    ctype = function()
      io.stderr:write('warning: deprecated type \'NONE\', use \'VOID\' instead\r\n');
      return gtypes.VOID.ctype();
    end,
    artype = function()
      io.stderr:write('warning: deprecated type \'NONE\', use \'VOID\' instead\r\n');
      return gtypes.VOID.artype();
    end,
    peek = function()
      io.stderr:write('warning: deprecated type \'NONE\', use \'VOID\' instead\r\n');
      return gtypes.VOID.peek();
    end,
    set = function()
      io.stderr:write('warning: deprecated type \'NONE\', use \'VOID\' instead\r\n');
      return gtypes.VOID.set();
    end,
  },
  BOOLEAN = {
    ctype = function()
      return 'gboolean';
    end,
    peek = function()
      return peek_prefix .. 'boolean';
    end,
    set = function()
      return set_prefix .. 'boolean';
    end,
  },
  BOOL = {
    ctype = function()
      io.stderr:write('warning: deprecated type \'BOOL\', use \'BOOLEAN\' instead\r\n');
      return gtypes.BOOLEAN.ctype();
    end,
    artype = function()
      io.stderr:write('warning: deprecated type \'BOOL\', use \'BOOLEAN\' instead\r\n');
      return gtypes.BOOLEAN.artype();
    end,
    peek = function()
      io.stderr:write('warning: deprecated type \'BOOL\', use \'BOOLEAN\' instead\r\n');
      return gtypes.BOOLEAN.peek();
    end,
    set = function()
      io.stderr:write('warning: deprecated type \'BOOL\', use \'BOOLEAN\' instead\r\n');
      return set_prefix .. 'boolean';
    end,
  },
  CHAR = {
    ctype = function()
      return 'gchar';
    end,
    artype = 'INT';
    peek = function()
      return peek_prefix .. 'char';
    end,
    set = function()
      return set_prefix .. 'schar';
    end,
  },
  UCHAR = {
    ctype = function()
      return 'gchar';
    end,
    artype = 'UINT';
    peek = function()
      return peek_prefix .. 'char';
    end,
    set = function()
      return set_prefix .. 'uchar';
    end,
  },
  INT = {
    ctype = function()
      return 'gint';
    end,
    peek = function()
      return peek_prefix .. 'int';
    end,
    set = function()
      return set_prefix .. 'int';
    end,
  },
  UINT = {
    ctype = function()
      return 'guint';
    end,
    peek = function()
      return peek_prefix .. 'uint';
    end,
    set = function()
      return set_prefix .. 'uint';
    end,
  },
  LONG = {
    ctype = function()
      return 'glong';
    end,
    peek = function()
      return peek_prefix .. 'long';
    end,
    set = function()
      return set_prefix .. 'long';
    end,
  },
  ULONG = {
    ctype = function()
      return 'gulong';
    end,
    peek = function()
      return peek_prefix .. 'ulong';
    end,
    set = function()
      return set_prefix .. 'ulong';
    end,
  },
  INT64 = {
    ctype = function()
      return 'gint64';
    end,
    peek = function()
      return peek_prefix .. 'int64';
    end,
    set = function()
      return set_prefix .. 'int64';
    end,
  },
  UINT64 = {
    ctype = function()
      return 'guint64';
    end,
    peek = function()
      return peek_prefix .. 'uint64';
    end,
    set = function()
      return set_prefix .. 'uint64';
    end,
  },
  ENUM = {
    ctype = function()
      return 'gint';
    end,
    peek = function()
      return peek_prefix .. 'enum';
    end,
    set = function()
      return set_prefix .. 'enum';
    end,
  },
  FLAGS = {
    ctype = function()
      return 'guint';
    end,
    peek = function()
      return peek_prefix .. 'flags';
    end,
    set = function()
      return set_prefix .. 'flags';
    end,
  },
  FLOAT = {
    ctype = function()
      return 'gfloat';
    end,
    artype = 'DOUBLE',
    peek = function()
      return peek_prefix .. 'float';
    end,
    set = function()
      return set_prefix .. 'float';
    end,
  },
  DOUBLE = {
    ctype = function()
      return 'gdouble';
    end,
    peek = function()
      return peek_prefix .. 'double';
    end,
    set = function()
      return set_prefix .. 'double';
    end,
  },
  STRING = {
    ctype = function()
      return 'gchar*';
    end,
    peek = function()
      return peek_prefix .. 'string';
    end,
    set = function()
      return take_prefix .. 'string';
    end,
  },
  BOXED = {
    ctype = function()
      return 'gpointer';
    end,
    peek = function()
      return peek_prefix .. 'boxed';
    end,
    set = function()
      return take_prefix .. 'boxed';
    end,
  },
  PARAM = {
    ctype = function()
      return 'gpointer';
    end,
    peek = function()
      return peek_prefix .. 'param';
    end,
    set = function()
      return take_prefix .. 'param';
    end,
  },
  POINTER = {
    ctype = function()
      return 'gpointer';
    end,
    peek = function()
      return peek_prefix .. 'pointer';
    end,
    set = function()
      return set_prefix .. 'pointer';
    end,
  },
  OBJECT = {
    ctype = function()
      return 'gpointer';
    end,
    peek = function()
      return peek_prefix .. 'object';
    end,
    set = function()
      return take_prefix .. 'object';
    end,
  },
  VARIANT = {
    ctype = function()
      return 'gpointer';
    end,
    peek = function()
      return peek_prefix .. 'variant';
    end,
    set = function()
      return take_prefix .. 'variant';
    end,
  },

--[[
--
-- Extension types
--
--]]

  INSTANCE = {
    ctype = function()
      return gtypes.OBJECT.ctype();
    end,
    peek = function()
      return gtypes.OBJECT.peek();
    end,
    set = function()
      return gtypes.OBJECT.set();
    end,
  },
};

for _, gtype in pairs(gtypes) do
  if(gtype.artype == nil) then
    gtype.artype = gtype.ctype;
  end
end

for _, gtype in pairs(gtypes) do
  for name, func in pairs(gtype) do
    if(type(func) == 'string') then
      gtype[name] = gtypes[func][name];
    end
  end
end

return gtypes;
