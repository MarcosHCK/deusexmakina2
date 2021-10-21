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
local build = require('build')
local event = require('event')
local ds = require('ds')
local glm = require('glm')
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
  local shader, skybox, font, text, error
  local GFile = lgi.Gio.File
  local Ds = lgi.Ds

--[[
--
-- Log domain
--
--]]

  ds.log = lgi.log.domain(build.PACKAGE)

--[[
--
-- Objects
--
--]]

  ds.Pkg = lgi.package(build.PACKAGE)
  ds.Pkg = lgi[build.PACKAGE]

  local objectsdir = GFile.new_for_path(ds.OBJECTSDIR)
  local enum = objectsdir:enumerate_children('standard::name', 'NONE')

  repeat
    local info, error = enum:next_file()
    if(info ~= nil) then
      local name = info:get_name();
      local objectfile = objectsdir:get_child(name);
      local success, reason = pcall(dofile, objectfile:peek_path())
      if(success == false) then
        ds.log.warning(reason);
      end
    else
      lgi.assert(error == nil, error)
      break;
    end
  until(false)

--[[
--
-- Model shaders
--
--]]

  shader, error =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/model_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/model_fs.glsl'),
    nil,
    nil,
    cancellable);
  assert(error == nil, error)

  pipeline:register_shader('model', ds.priority.default, shader);

--[[
--
-- Skybox shaders
--
--]]

  shader, error =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/skybox_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/skybox_fs.glsl'),
    nil,
    nil,
    cancellable);
  assert(error == nil, error)

  pipeline:register_shader('skybox', ds.priority.higher, shader);

--[[
--
-- Text shaders
--
--]]

  shader, error =
  Ds.Shader.new_from_files(
    GFile.new_for_path(ds.GFXDIR .. '/text_vs.glsl'),
    GFile.new_for_path(ds.GFXDIR .. '/text_fs.glsl'),
    nil,
    nil,
    cancellable);
  assert(error == nil, error)

  pipeline:register_shader('text', ds.priority.lower, shader);

--[[
--
-- Skybox
--
--]]

  skybox, error =
  Ds.Skybox.new(
    GFile.new_for_path(ds.ASSETSDIR .. '/skybox/'),
    '%s.dds',
    cancellable);
  assert(error == nil, error)

  pipeline:append_object('skybox', ds.priority.default, skybox);

--[[
--
-- Font
--
--]]

  font, error =
  Ds.Font.new(
    GFile.new_for_path(ds.ASSETSDIR .. '/Unknown.ttf'),
    13,
    nil,
    cancellable);
  assert(error == nil, error)

--[[
--
-- Text renderer
--
--]]

  text, error =
  Ds.Text.new(font);
  assert(error == nil, error)

  pipeline:append_object('text', ds.priority.default, text);
  text:print(nil, build.PACKAGE_STRING, 2, 600 - 12 - 2, cancellable);

--[[
--
-- Update pipeline
--
--]]

  local _, error =
  pipeline:update(cancellable);
  assert(error == nil, error)

--[[
--
-- Listen for events
--
--]]

  event.listen('mouse_motion', function(_, x, y, xrel, yrel)
    renderer:look(xrel, yrel);
  end);
end
