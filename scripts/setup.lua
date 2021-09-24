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
local cancellable = ...;

-- Check debug flag
local debug_ = _ENV["DS_DEBUG"] == 'true';

-- Load skybox shader
do
  local shader, skybox, font;
  local error = ds.type.DsError();

--[[
--
-- Model shaders
--
--]]

  shader =
  ds.type.DsShader.new_simple(
    ds.GFXDIR .. '/model_vs.glsl',
    ds.GFXDIR .. '/model_fs.glsl',
    nil,
    cancellable,
    error:ref());
  error:check();

  pipeline:register_shader('model', 0, shader);

--[[
--
-- Skybox shaders
--
--]]

  shader =
  ds.type.DsShader.new_simple(
    ds.GFXDIR .. '/skybox_vs.glsl',
    ds.GFXDIR .. '/skybox_fs.glsl',
    nil,
    cancellable,
    error:ref());
  error:check();

  pipeline:register_shader('skybox', -200, shader);

--[[
--
-- Text shaders
--
--]]

  shader =
  ds.type.DsShader.new_simple(
    ds.GFXDIR .. '/text_vs.glsl',
    ds.GFXDIR .. '/text_fs.glsl',
    nil,
    cancellable,
    error:ref());
  error:check();

  pipeline:register_shader('text', -100, shader);

--[[
--
-- Skybox
--
--]]

  skybox = ds.type.DsSkybox.new_simple(
    ds.ASSETSDIR .. '/skybox/',
    '%s.dds',
    cancellable,
    error:ref());
  error:check();

  pipeline:append_object('skybox', -100, skybox);

--[[
--
-- Font
--
--]]

  font = ds.type.DsFont.new_simple(
    ds.ASSETSDIR .. '/WickedGrit.ttf',
    12,
    cancellable,
    error:ref());
  error:check();

  print(font);
end
