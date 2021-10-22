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
local ds = require('ds')
local event = require('event')
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
      local success, reason = pcall(require, 'objects.' .. (name:gsub('.lu[ac]', '')))
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

  local shader, error =
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

  local shader, error =
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

  local shader, error =
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

  local function mkskybox()
    local skybox, error =
    Ds.Skybox.new(
      GFile.new_for_path(ds.ASSETSDIR .. '/skybox/'),
      '%s.dds',
      cancellable);
    assert(error == nil, error)

    pipeline:append_object('skybox', ds.priority.default, skybox);
  end
  lgi.Gio.Async.start(mkskybox)()

--[[
--
-- Font
--
--]]

  local font, error =
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

  local text, error =
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

  local w_ = string.byte('w')
  local a_ = string.byte('a')
  local d_ = string.byte('d')
  local s_ = string.byte('s')

  event.listen('Mouse.Motion', function(_, x, y, xrel, yrel)
    renderer:look(xrel, yrel);
  end);
  event.listen('Key.Down', function(_, sym, repeat_)
    if(sym == w_) then
      renderer:move(   0,    0,  0.1, true);
    elseif(sym == a_) then
      renderer:move(-0.1,    0,    0, true);
    elseif(sym == d_) then
      renderer:move( 0.1,    0,    0, true);
    elseif(sym == s_) then
      renderer:move(   0,    0, -0.1, true);
    end
  end)
end
