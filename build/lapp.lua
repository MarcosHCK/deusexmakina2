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

--[[
--
-- This file is a heavy customized version of the original package
-- to allow it to run with minilua lua engine shipped with LuaJIT.
--
--]]

--[[
-- Copyright 2020-2021 MarcosHCK
--  This file is part of libLApp.
--
--  libLApp is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  libLApp is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with libLApp.  If not, see <http://www.gnu.org/licenses/>.
]]
local lapp = {};
lapp.version = "libLApp v1.3.3.0 Mod";

--
-- Bootstrap
--

local text;
do
  if(type(checkArg) ~= 'function') then
    local function check(have, want, ...)
      if not want then
        return false
      else
        return have == want or check(have, ...)
      end
    end

    function checkArg(n, have, ...)
      have = type(have)
      if not check(have, ...) then
        local msg = string.format("bad argument #%d (%s expected, got %s)", n, table.concat({...}, " or "), have)
        error(msg, 3)
      end
    end
  end

  local success, text_ = pcall(require, 'text');
  if(success == true) then
    text = text_;
  else
    text = {
      padRight = function(value, spaces)
        return value .. string.rep(' ', spaces - #value);
      end,
    };
  end

  if(table.unpack == nil) then
    table.unpack = unpack;
  end
end

--
-- Help handler
--

local help_opt = {
  short = '-h',
  long = '--help',
  type = 'none',
  callback = function(app)
    local arguments = app.arguments;
    local options = app.options;

    io.stdout:write(string.format('%s\r\nOptions:', app.arg_0 or app.name));
    for _, argument in ipairs(arguments) do
      local arguments_string = argument.name;
      if(argument.optional) then
        arguments_string = string.format(' [%s]', arguments_string);
      else
        arguments_string = ' ' .. arguments_string;
      end

      io.write(arguments_string);
    end

    io.write('\r\n');
    local paddmax = 1;
    for _, option in ipairs(options) do
      local names = table.concat({option.short or '', option.long or ''}, ', ');
      local len = #names;
      if(len > paddmax) then
        paddmax = len;
      end
    end

    paddmax = paddmax + (10 - paddmax % 10);

    for _, option in ipairs(options) do
      io.stdout:write(text.padRight('', 10));

      if(option.short) then
        local names = table.concat({option.short, option.long}, ', ');
        io.stdout:write(text.padRight(names, paddmax));
      else
        io.stdout:write(text.padRight(string.rep(' ', 4) .. option.long, paddmax));
      end

      local description = option.description;
      if(description) then
        io.stdout:write(text.padRight(description, paddmax))
      end

        io.stdout:write('\n');
    end

    if(#app.notes > 0) then
      io.stdout:write('Note' .. (#app.notes > 1 and 's' or '') .. ':\r\n');
      for _, note in ipairs(app.notes) do
        io.stdout:write('- ' .. note .. '\r\n');
      end
    end

    os.exit(0);
  end,
};

--
-- Library functions
--

function lapp.add_argument(app, name, optional)
  checkArg(2, name, 'string');
  checkArg(3, optional, 'boolean', 'nil');

  local argument = {
    name = name,
    optional = (optional == true),
  };

  table.insert(app.arguments, argument);
return true;
end

function lapp.add_arguments(app, arguments)
  checkArg(2, arguments, 'table');

  local add_argument = lapp.add_argument;
  for _, argument in ipairs(arguments) do
    assert(type(argument) == 'table');
    local success = add_argument(app, table.unpack(option));
    if(not success) then
      return success;
    end
  end
return true;
end

function lapp.add_option(app, short, long, description, type_, callback)
  checkArg(2, short, 'string', long ~= nil and 'nil');
  checkArg(3, long, 'string', short ~= nil and 'nil');
  checkArg(4, description, 'string', 'nil');
  checkArg(5, type_, 'string', 'nil');
  checkArg(6, callback, 'function', 'nil');

  type_ = type_ or 'none';
  assert(
      type_ == 'none' or
      type_ == 'string' or
      type_ == 'number' or
      type_ == 'file' or
      type_ == 'file-create'
    , 'Invalid option type');

  local option = {
    short = short and ('-' .. short),
    long = long and ('--' .. long),
    type = type_,
    callback = callback,
    description = description,
  };

  table.insert(app.options, option);
return true;
end

function lapp.add_options(app, options)
  checkArg(2, options, 'table');

  local add_option = lapp.add_option;
  for _, option in ipairs(options) do
    assert(type(option) == 'table');
    local success = add_option(app, table.unpack(option));
    if(not success) then
      return success;
    end
  end
return true;
end

function lapp.add_note(app, note)
  checkArg(2, note, 'string');
  table.insert(app.notes, note);
return true;
end

function lapp.add_notes(app, notes)
  checkArg(2, notes, 'table');

  for _, note in ipairs(notes) do
    assert(type(note) == 'string');
    local success = lapp.add_note(app, note);
    if(not success) then
      return success;
    end
  end
end

local function search_option(options, option_name)
  local long = option_name:match('^%-%-') and 'long' or 'short';
  for _, option in ipairs(options) do
    if(option[long] == option_name) then
      return option;
    end
  end
end

local function check_arg(option, option_name, arg)
  if(type(arg) ~= 'string') then
    return false, 'Option ' .. option_name .. ' takes an argument';
  elseif(option.type == 'string') then
    option.arg = arg;
  elseif(option.type == 'file') then
    if(not require('filesystem').exists(arg)) then
      return false, 'File ' .. arg .. ' doesn\'t exists';
    end
  elseif(option.type == 'number') then
    local number = tonumber(arg);
    if(type(number) ~= 'number') then
      return false, 'Option ' .. option_name .. 'takes a number argument';
    else
      option.arg = number;
    end
  end
return true;
end

function lapp.run(app, ...)
  local files = {};
  local next_, cmd, key, option_name = ipairs({...});
  app.arg_0 = cmd[0];

  local pending_calls = {};

  while(key) do
    key, option_name = next_(cmd, key);
    if(type(option_name) == 'string') then
      if(option_name:match('^%-%-?.')) then

        local function parse_option(option_name)
          local option_name_ = option_name:match('([^%=]*)');
          local option_arg_ = option_name:match('%=(.*)');
          local option = search_option(app.options, option_name_);
          if(option) then
            local arg;
            option.arg = nil;

            if(option.type ~= 'none') then
              if(option_arg_ ~= nil) then
                local success, reason = check_arg(option, option_name_, option_arg_);
                if(not success) then
                  return success, reason;
                end
              else
                key, arg = next_(cmd, key);
                local success, reason = check_arg(option, option_name_, arg);
                if(not success) then
                  return success, reason;
                end
              end
            else
              if(option_arg_ ~= nil) then
                return false, 'Option \'' .. option_name_ .. '\' doesn\'t take any arguments';
              else
                option.arg = true;
              end
            end

            if(option.callback) then
              table.insert(pending_calls, {
                [0] = option.callback,
                [1] = app,
                [2] = args,
              });
            end
          else
            return false, 'Unhandled option: ' .. option_name_;
          end
        return true;
        end

        local islong = (option_name:sub(2, 2) == '-');
        if(islong) then
          local ret, reason = parse_option(option_name);
          if(ret ~= true) then
            return ret, reason;
          end
        else
          for i = 2, #option_name, 1 do
            local bit = option_name:sub(i, i);
            local ret, reason = parse_option('-' .. bit);
            if(ret ~= true) then
              return ret, reason;
            end
          end
        end
      else
        table.insert(files, option_name);
      end
    end
  end

  for _, pending in ipairs(pending_calls) do
    local func = pending[0];
    pending[0] = nil;

    func(table.unpack(pending));
  end
return true, app.main(app, files);
end

local function select2(some, ...)
return ...;
end

function lapp.run_s(app, ...)
  local result = {lapp.run(app, ...)};
  if(result[1]) then
    return select2(table.unpack(result));
  else
    error(select2(table.unpack(result)));
  end
end

function lapp.getopt(app, option_name_)
  if(option_name_:match('^%-%-?.')) then
    local option = search_option(app.options, option_name_);
    if(option) then
      return option.arg;
    end
  else
    local option_names = {'-' .. option_name_, '--' .. option_name_};
    for _, option_name in ipairs(option_names) do
      local option = search_option(app.options, option_name);
      if(option) then
        return option.arg;
      end
    end
  end
end

function lapp.assertopt(app, option_name_)
  local value = lapp.getopt(app, option_name_);
  if(not value) then
    error(
    string.format('%s: option \'%s\' requires an argument\r\n' ..
                  'Try \'%s --help\' for more information\r\n',
                  app.name,
                  option_name_,
                  app.name
                  ), 1);
  end
return value;
end

function lapp.assertarg(app, arg)
  if(not arg) then
    error(
    string.format('%s: requires and argument\r\n' ..
                  'Try \'%s --help\' for more information\r\n',
                  app.name,
                  app.name
                  ), 1);
  end
return arg;
end

function lapp.assert(app, condition, message)
  if(condition ~= true) then
    error(
      string.format('%s: fatal error: %s',
        app.name, message), 2);
  end
end

function lapp.new(name, main_)
  return {
    arguments = {},
    main = main_,
    name = name,
    options = {
      [1] = help_opt,
    },
    notes = {},

    add_argument = lapp.add_argument,
    add_arguments = lapp.add_arguments,
    add_option = lapp.add_option,
    add_options = lapp.add_options,
    add_note = lapp.add_note,
    add_notes = lapp.add_notes,
    run_s = lapp.run_s,
    getopt = lapp.getopt,
    assertopt = lapp.assertopt,
    assertarg = lapp.assertarg,
    assert = lapp.assert,
  };
end

--
-- Library
--
return lapp;
