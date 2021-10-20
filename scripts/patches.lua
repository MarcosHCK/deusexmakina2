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
local ds = require('ds');
local scriptsdir = ...;

--[[
--
-- This file holds some code which extends built-in Lua standard library,
-- as well as this project's own API library. Here also are placed some
-- code which needs to be run as early as possible at startup, like some
-- Lua compatibility layer, or library paths initialization. 
--
--]]

--
-- Create a php-like ENV table
--

do
  local cache = {};
  _G._ENV = setmetatable({}, {
    __index = function(_ENV, key)
      local env = cache[key];
      if(env == nil) then
        env = os.getenv(key);
        cache[key] = env;
      end
    return env;
    end,
    __newindex = function(_ENV, key, env)
      if(cache[key] ~= env) then
        os.setenv(key, env);
        cache[key] = env;
      end
    end,
  });
end

--
-- Add checkArg facility (useful, idea taken from OpenComputers)
--

local function check(have, want, ...)
  if(not want) then
    return false;
  else
    return have == want or check(have, ...);
  end
end

function checkArg(n, have, ...)
  have = type(have);
  if(not check(have, ...)) then
    local msg = string.format('bad argument #%d (%s expected, got %s)', n, table.concat({...}, ' or '), have);
    error(msg, 3);
  end
end

--
-- Patch 'table.pack' and 'table.unpack' functions
-- which are absent on LuaJIT.
--

table.pack = table.pack or function(...) return {...} end
table.unpack = table.unpack or assert(_G.unpack)

--
-- Add custom module path
--

do
  local newpath = table.concat({
    scriptsdir .. '/?.lua',
    scriptsdir .. '/?/init.lua',
  }, ';');
  local newcpath = table.concat({
    scriptsdir .. '/?.so',
  }, ';');

  package.path =
  table.concat({package.path, newpath}, ';');
  package.cpath =
  table.concat({package.cpath, newcpath}, ';');
end

--
-- Overwrite priority table
--

do
  local higher = ds.priority.higher;
  local lower = ds.priority.lower;

  ds.priority.higher = nil;
  ds.priority.lower = nil;

  setmetatable(ds.priority,
  {
    __index = function(t, k)
      if(k == 'higher') then
        higher = higher + 1;
      return higher;
      elseif(k == 'lower') then
        lower = lower - 1;
      return lower;
      end
    end,
    __newindex = function()
      error('protected table');
    end,
  });
end

--
-- Initialize LGI
--

do
  local lgi = require('lgi')
  local core = require('lgi.core')
  local build = require('build')

  -- LGI doesn't do this itself (cool?)
  package.loaded.lgi = lgi

  local version = ('%s.%s'):format(build.PACKAGE_VERSION_MAYOR, build.PACKAGE_VERSION_MINOR)
  local Ds = core.gi.require('Ds', version, ds.GIRDIR)
  local Ds = lgi.require('Ds', version)

  local Matrix = Ds.Matrix
  local attribute = {}
  local glm = {}

  package.loaded.glm = glm
  Matrix._attribute = attribute

  local names =  {'vec2', 'vec3', 'vec4', 'mat2', 'mat3', 'mat4'}
  local floats = {    2,      3,      4,      4,      9,     16 }

  for i = 1, #names do
    local name = names[i]
    local float = floats[i]
    local field = name .. '_'

    glm[name] = function (...)
      local array = {...}
      for j = 1, float do
        checkArg(1, array[j], 'number')
      end

    return Matrix.new ({unpack(array, 1, float)})
    end

    attribute[name] = {
      get = function(matrix)
        return matrix.value[field]
      end
    }
  end
end

collectgarbage()
