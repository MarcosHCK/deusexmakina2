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
local event = {};
local handlers = {};
local lastEvent = 0;

--
-- Public API
--
function event.register(key, callback, times, opt_handlers)
  local handler =
  {
    key = key,
    times = times or 1,
    callback = callback,
  }

  opt_handlers = opt_handlers or handlers;

  local id = 0;
  repeat
    id = id + 1;
  until(not opt_handlers[id]);

  opt_handlers[id] = handler;
return id;
end

function event.unregister(key, callback)
  checkArg(1, key, 'string', 'number');
  if(type(key) == 'string') then
    checkArg(2, callback, 'function');
    for id, handler in pairs(handlers) do
      if(handler.key == key and handler.callback == callback) then
        handlers[id] = nil;
      end
    end
  else
    handlers[id] = nil;
  end
end

function event.listen(key, callback)
  return event.register(key, callback, math.huge);
end

function event.push(name, ...)
  checkArg(1, name, 'string');
  for id, handler in pairs(handlers) do
    if(handler.key == nil or handler.key == name) then
--
-- TTL collect
--
      handler.times = handler.times - 1;
      if((0 >= handler.times) and (handlers[id] == handler)) then
          handlers[id] = nil;
      end

--
-- Call
--
      local success, reason = pcall(handler.callback, name, ...);
      if(not success) then
        pcall(event.onError, reason);
      elseif(reason == false and handlers[id] == handler) then
        handlers[id] = nil;
      end
    end
  end
end

return event;
