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
local application, cancellable = ...
local event = require('event')
local ds = require('ds')
local lgi = require('lgi')

--[[
--
-- This file contains setup code for graphical engine, an therefore
-- has a huge impact on game initialization. Note that application
-- at this point is almost complete, which means almost all features
-- of this project's own Lua API library is ready to use.
--
--]]

--
-- Retrieve objects
--

application = application and lgi.Ds.Application(application)
cancellable = cancellable and lgi.Gio.Cancellable(cancellable)

--
-- Initialize things
--

do
  local pipeline = application.pipeline
  local renderer = application.renderer
  local shader, skybox, font, text
  local GFile = lgi.Gio.File
  local Ds = lgi.Ds

--[[
--
-- Model shaders
--
--]]

  shader =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/model_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/model_fs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/debug_gs.glsl'),
    nil,
    cancellable);

  pipeline:register_shader('model', ds.priority.default, shader);

--[[
--
-- Skybox shaders
--
--]]

  shader =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/skybox_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/skybox_fs.glsl'),
    nil,
    nil,
    cancellable);

  pipeline:register_shader('skybox', ds.priority.higher, shader);

--[[
--
-- Text shaders
--
--]]

  shader =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/text_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/text_fs.glsl'),
    nil,
    nil,
    cancellable);

  pipeline:register_shader('text', ds.priority.lower, shader);

--[[
--
-- Skybox
--
--]]

  skybox =
  Ds.Skybox.new(
    GFile.new_for_path(ds.ASSETSDIR .. '/skybox/'),
    '%s.dds',
    cancellable);

  --pipeline:append_object('skybox', ds.priority.default, skybox);

--[[
--
-- Font
--
--]]

  font =
  Ds.Font.new(
    GFile.new_for_path(ds.ASSETSDIR .. '/Unknown.ttf'),
    13,
    nil,
    cancellable);

--[[
--
-- Text renderer
--
--]]

  text =
  Ds.Text.new(font);
  pipeline:append_object('text', ds.priority.default, text);

  text:print(nil, require('build').PACKAGE_STRING, 2, 600 - 12 - 2, cancellable);

--[[
--
-- Test model
--
--]]

  do
    local vec3 = require('glm').vec3

    for i = 1, 3 do
      local model = Ds.ModelSingle.new(
        GFile.new_for_path(ds.ASSETSDIR),
        'backpack.obj',
        cancellable)

      local scale = vec3(
          0.08,
          0.08,
          0.08)
      local position = vec3(
          0.10 * i,
          0,
         -0.2)

      model:set_scale(scale.vec3)
      model:set_position(position.vec3)

      pipeline:append_object('model', 0, model)
    end
  end

--[[
--
-- Update pipeline
--
--]]

  pipeline:update(cancellable);

--[[
--
-- Listen for events
--
--]]

  event.listen('mouse_motion', function(_, x, y, xrel, yrel)
    renderer:look(xrel, yrel);
  end);
end
