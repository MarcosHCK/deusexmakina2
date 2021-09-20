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
local ds = require('ds');
local cancellable = ...;

-- Load skybox shader
do
  local shader = nil;
  local klass = ds.type.DsShader;
  local error = ds.type.DsError.new();

  if(_ENV["DS_DEBUG"] == 'true') then
    shader =
    klass.new_simple(
      ds.ABSTOPBUILDDIR .. '/gfx/' .. 'skybox_vs.glsl',
      ds.ABSTOPBUILDDIR .. '/gfx/' .. 'skybox_fs.glsl',
      nil,
      cancellable,
      error:ref());
  else
    shader =
    klass.new_simple(
      ds.GFXDIR .. 'skybox_vs.glsl',
      ds.GFXDIR .. 'skybox_fs.glsl',
      nil,
      cancellable,
      error:ref());
  end

  error:check();
end

print(shader);
shader = nil;
