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

-- Replacement for customized Lua, which lacks the package library.
do
  local prefix = '';
  function require(name)
    local fp = assert(io.open(prefix .. name .. '.lua'));
    local s = fp:read("*a");
    assert(fp:close());
    return assert(loadstring(s, '@' .. name .. '.lua'))();
  end

  local arg = arg or args or {...};
  if(arg and arg[0]) then
    prefix = arg[0]:match('^(.*[/\\])');
    if(package and prefix) then
      package.path = prefix .. '?.lua;' .. package.path;
    end
  end
end

local gtypes = require('gtypes');
local lapp = require('lapp');

local prologue = [[
#include <glib-object.h>

#ifdef DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_schar (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#define g_marshal_value_peek_variant(v)  g_value_get_variant (v)
#else // DEBUG
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          This macros are copy as-in from glib-genmarshal output,
 *          an may be stable across minor builds, but can change
 *          with next mayor release.
 */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_variant(v)  (v)->data[0].v_pointer
#endif // DEBUG

]];

local function functionname(app, args, return_, va_list)
  local name = '';
  local chuck = '';

  if(app:getopt('internal')) then
    name = name .. 'G_GNUC_INTERNAL\r\n';
  end

  name = name .. 'void\r\n';

  local prefix = app:getopt('prefix');
  chuck = chuck .. ((prefix and prefix .. '_' or 'ds_dsclosure_marshal_') .. return_ .. '_');
  for _, arg in ipairs(args) do
    chuck = chuck .. '_' .. arg;
  end

  if(va_list == true) then
    chuck = chuck .. 'v ';
  else
    chuck = chuck .. ' ';
  end
return name .. chuck, #chuck;
end

local function prototype(app, output, args, return_)
  local name, pad = functionname(app, args, return_, false);

  output:write(name .. [[
(GClosure     *closure,
]] .. string.rep(' ', pad) .. [[
 GValue       *return_value,
]] .. string.rep(' ', pad) .. [[
 guint         n_param_values,
]] .. string.rep(' ', pad) .. [[
 const GValue *param_values,
]] .. string.rep(' ', pad) .. [[
 gpointer      invocation_hint,
]] .. string.rep(' ', pad) .. [[
 gpointer      marshal_data)]]);
end

local function vaprototype(app, output, args, return_)
  local name, pad = functionname(app, args, return_, true);

  output:write(name .. [[
(GClosure *closure,
]] .. string.rep(' ', pad) .. [[
 GValue   *return_value,
]] .. string.rep(' ', pad) .. [[
 gpointer  instance,
]] .. string.rep(' ', pad) .. [[
 va_list   args,
]] .. string.rep(' ', pad) .. [[
 gpointer  marshal_data,
]] .. string.rep(' ', pad) .. [[
 int       n_params,
]] .. string.rep(' ', pad) .. [[
 GType    *param_types)]]);
end

local function mkmarshal(app, output, args, return_, proto, body)
  if(proto == true) then
    prototype(app, output, args, return_);
    output:write(';\r\n');
  end
  if(body == true) then
    prototype(app, output, args, return_);
    local suffix = return_ .. '_';
    for _, arg in ipairs(args) do
      suffix = suffix .. '_' .. arg;
    end

    local function gettypes()
      local out = '';
      local i = 1;
      if(#args == 1 and args[1] == 'VOID') then
        return 'void';
      end

      for _, arg in ipairs(args) do
        out = out .. string.format('%s data%i, ', gtypes[arg].ctype(), i);
        i = i + 1;
      end
    return out:gsub(', $', '');
    end

    local function getinvoke()
      local out = '';
      local i = 1;
      if(#args == 1 and args[1] == 'VOID') then
        return out;
      end

      for _, arg in ipairs(args) do
        out = out .. string.format('%s (param_values + %i),\r\n%s', gtypes[arg].peek(), i - 1, '   ');
        i = i + 1;
      end
    return out:gsub(',[%s]*$', '');
    end

    local typename = 'DsMarshalFunc_' .. suffix;
    local return_ctype = gtypes[return_].ctype();

    output:write('\r\n' .. [[
{
  typedef ]] .. return_ctype .. [[ (*]] .. typename .. [[) (]] .. gettypes() .. [[);

  GCClosure* cc = (GCClosure*) closure;
  ]] .. typename .. [[ callback;
  ]] .. (return_ ~= 'VOID' and (return_ctype .. ' return_;' .. '\r\n') or '') .. [[

  ]] .. (return_ ~= 'VOID' and ('g_return_if_fail(return_value != NULL);' .. '\r\n  ') or '') .. [[
g_return_if_fail(n_param_values == ]] .. (#args) .. [[);

  callback = (]] .. typename .. [[) (marshal_data ? marshal_data : cc->callback);

  ]] .. (return_ ~= 'VOID' and ('return_ = ') or '') .. [[callback
  (]] .. getinvoke() .. [[);

  ]] .. (return_ ~= 'VOID' and (gtypes[return_].set() .. ' (return_value, return_);\r\n') or '') .. [[
}
]]);
  end
  output:write('\r\n');
end

local function mkvamarshal(app, output, args, return_, proto, body)
  if(proto == true) then
    vaprototype(app, output, args, return_);
    output:write(';\r\n');
  end
  if(body == true) then
    vaprototype(app, output, args, return_);
    local suffix = return_ .. '_';
    for _, arg in ipairs(args) do
      suffix = suffix .. '_' .. arg;
    end

    local function gettypes()
      local out = '';
      local i = 1;
      if(#args == 1 and args[1] == 'VOID') then
        return 'void';
      end

      for _, arg in ipairs(args) do
        out = out .. string.format('%s data%i, ', gtypes[arg].ctype(), i);
        i = i + 1;
      end
    return out:gsub(', $', '');
    end

    local function getlist()
      local out = '';
      local prev = '';
      local i = 1;
      if(#args == 1 and args[1] == 'VOID') then
        return 'void';
      end

      for _, arg in ipairs(args) do
        out = out .. string.format('%sdata%i = (%s) va_arg(args_list, %s);\r\n', prev, i, gtypes[arg].ctype(), gtypes[arg].artype());
        prev = '  ';
        i = i + 1;
      end
    return out:gsub(', $', '');
    end

    local function getinvoke()
      local out = '';
      local i = 1;
      if(#args == 1 and args[1] == 'VOID') then
        return out;
      end

      for _, arg in ipairs(args) do
        out = out .. string.format('data%i, ', i);
        i = i + 1;
      end
    return out:gsub(',[%s]*$', '');
    end

    local typename = 'DsMarshalFunc_' .. suffix;
    local return_ctype = gtypes[return_].ctype();

    output:write('\r\n' .. [[
{
  typedef ]] .. return_ctype .. [[ (*]] .. typename .. [[) (]] .. gettypes() .. [[);

  GCClosure* cc = (GCClosure*) closure;
  ]] .. typename .. [[ callback;
  ]] .. (return_ ~= 'VOID' and (return_ctype .. ' return_;' .. '\r\n') or '') .. [[
  ]] .. ((#args == 1 and args[1] == 'VOID') and '' or gettypes():gsub(',[%s]*', ';\r\n  ') .. ';\r\n') .. '\r\n' .. [[
  g_return_if_fail(n_params == ]] .. (#args) .. [[);

  va_list args_list;
  G_VA_COPY(args_list, args);
  ]] .. ((#args == 1 and args[1] == 'VOID') and '' or getlist()) .. [[
  va_end(args_list);

  callback = (]] .. typename .. [[) (marshal_data ? marshal_data : cc->callback);

  ]] .. (return_ ~= 'VOID' and ('return_ = ') or '') .. [[callback
  (]] .. getinvoke() .. [[);

  ]] .. (return_ ~= 'VOID' and (gtypes[return_].set() .. ' (return_value, return_);\r\n') or '') .. [[
}
]]);
  end
  output:write('\r\n');
end

local function process(app, input, output, name)
  local i = 0;
  repeat
    local line = input:read('*l');
    if(line ~= nil) then
      local s = line:sub(1, 1);
      if(s ~= '#' and #line > 0) then
        local return_ = line:match('(.*)%:');
        local args_string = line:match('%:(.*)');
        local args = {};

        if(return_ ~= nil and args_string ~= nil) then
          args_string:gsub('[^%,]+', function(arg) table.insert(args, arg); end);
          output:write(string.format('/* %s (%s: %i) */\r\n', line, name, i));

          if(app:getopt('v') == true) then
            io.stdout:write(string.format('generation marshal for \'%s\'\r\n', line));
          end
          mkmarshal(app, output, args, return_, app:getopt('H') or app:getopt('prototypes'), app:getopt('C'));

          if(app:getopt('valist-marshallers') == true) then
            if(app:getopt('v') == true) then
              io.stdout:write(string.format('generation va_list marshal for \'%s\'\r\n', line));
            end
            mkvamarshal(app, output, args, return_, app:getopt('H') or app:getopt('prototypes'), app:getopt('C'));
          end
        else
          error(string.format('Malformed line %i: \'%s\'', i, line));
        end
      end
    else
      break;
    end
    i = i + 1;
  until(false);
  io.stdout:write('info: processed file \'' .. name .. '\'\r\n');
end

local app = lapp.new('mkmershal', function(app, files)
  app:assert(#files >= 1, 'no input file');

  local outfile = app:getopt('output');
  app:assert(outfile ~= nil, 'Specify an output file');
  local output = io.open(outfile, 'w');
  app:assert(output ~= nil, string.format('Can\'t open file \'%s\'', outfile));

  app:assert(app:getopt('C') or app:getopt('H'), 'Use code or header flags');
  app:assert(app:getopt('C') ~= app:getopt('H'), 'Use code or header flags, not both');

  output:write([[
/*
 * This file is generated by mkmarshal.lua, do not modify it.
 * This code is licensed under the same license as deusexmakina2.
 * Note that it links to GLib, so must comply with the LGPL linking clauses.
 *
 */
]]);

  if(app:getopt('include-header') ~= nil and app:getopt('C') ~= nil) then
    output:write('#include <' .. app:getopt('include-header') .. '>\r\n');
  end
  if(app:getopt('valist-marshallers')~= nil) then
    output:write('#include <string.h>\r\n');
  end

  output:write(prologue);

  for _, file in pairs(files) do
    local input;
    if(file ~= '-') then
      input = io.open(file, 'r');
    else
      input = io.stdin;
    end

    app:assert(input ~= nil, string.format('Can\'t open file \'%s\'', file));
    process(app, input, output, file);
    input:close();
  end

  output:close();
end);

app:add_options({
  {'C', 'body', 'Generate C code'},
  {'H', 'header', 'Generate C header'},
  {nil, 'prefix', 'Specify marshaller prefix', 'string'},
  {'o', 'output', 'Write output into the specified file', 'string'},
  {nil, 'internal', 'Mark generated marshallers as internal'},
  {nil, 'valist-marshallers', 'Generate va_list marshaller as well'},
  {'I', 'include-header', 'Include the specified header in the body', 'string'},
  {nil, 'pragma-once', 'Use \'pragme one\' as the inclusion guard'},
  {'D', 'define', 'Pre-processor define', 'string'},
  {'U', 'undefine', 'Pre-processor undefine', 'string'},
  {nil, 'prototypes', 'Generate the marshellers prototypes along with C code'},
  {'q', 'quiet', 'Only print warning an errors', nil},
  {'v', 'verbose', 'Print verbose messages', nil},
  {'V', 'version', 'Print version information', nil},
});

app:run_s(...);