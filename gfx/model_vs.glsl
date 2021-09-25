#version 330 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

out vec2 TexCoords;

uniform mat4 a_mvp;

void main()
{
  TexCoords = a_TexCoords;    
  gl_Position = a_mvp * vec4(a_Pos, 1.0);
}
