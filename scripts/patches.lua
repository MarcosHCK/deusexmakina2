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
]]
local scriptsdir = ...;

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

if(table.pack == nil) then
  table.pack = function(...)
    return {...};
  end
end

if(table.unpack == nil) then
  table.unpack = assert(_G.unpack);
end

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
