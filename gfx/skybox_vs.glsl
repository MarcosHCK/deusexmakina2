/*  Copyright 2021-2022 MarcosHCK
 *  This file is part of deusexmakina2.
 *
 *  deusexmakina2 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  deusexmakina2 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with deusexmakina2.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#version 330 core
out vec3 TexCoords;

layout (location = 0) in vec3 a_Pos;

uniform mat4 a_mvp;
uniform mat4 a_jvp;

void main()
{
  TexCoords = a_Pos;
  vec4 pos = a_jvp * vec4(a_Pos, 1.0);
  gl_Position = pos.xyzw;
}
