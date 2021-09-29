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
local application, cancellable = ...;
local event = require('event');
local ds = require('ds');

--[[
--
-- This file contains setup code for graphical engine, an therefore
-- has a huge impact on game initialization. Note that application
-- at this point is almost complete, which means almost all features
-- of this project's own Lua API library is ready to use.
--
--]]

-- Check debug flag
local debug_ = _ENV["DS_DEBUG"] == 'true';

-- Load skybox shader
do
  local pipeline = application.pipeline;
  local shader, skybox, font, text;
  local tmp_err = ds.type.DsError();

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
    nil,
    cancellable,
    tmp_err:ref());
  tmp_err:check();

  pipeline:register_shader('model', ds.priority.default, shader);

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
    nil,
    cancellable,
    tmp_err:ref());
  tmp_err:check();

  pipeline:register_shader('skybox', ds.priority.higher, shader);

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
    nil,
    cancellable,
    tmp_err:ref());
  tmp_err:check();

  pipeline:register_shader('text', ds.priority.lower, shader);

--[[
--
-- Skybox
--
--]]

  skybox = ds.type.DsSkybox.new_simple(
    ds.ASSETSDIR .. '/skybox/',
    '%s.dds',
    cancellable,
    tmp_err:ref());
  tmp_err:check();

  pipeline:append_object('skybox', ds.priority.default, skybox);

--[[
--
-- Font
--
--]]

  font = ds.type.DsFont.new_simple(
    ds.ASSETSDIR .. '/Unknown.ttf',
    13,
    nil,
    cancellable,
    tmp_err:ref());
  tmp_err:check();

--[[
--
-- Text renderer
--
--]]

  text = ds.type.DsText.new(font);
  pipeline:append_object('text', ds.priority.default, text);

  text:print(nil, require('build').PACKAGE_STRING, 2, 600 - 12 - 2, cancellable, tmp_err:ref());
  tmp_err:check();

--[[
--
-- Update pipeline
--
--]]

  pipeline:update(cancellable, tmp_err:ref());
  tmp_err:check();
end

-- Register default event tmp_err handler
do
  function event.onError(reason)
    if(type(reason) == 'string') then
      io.stderr:write(string.format('%s\r\n', reason));
    else
      io.stderr:write(string.format('Unsupported tmp_err reasoning of type \'%s\'\r\n', type(reason)));
    end
  end
end
