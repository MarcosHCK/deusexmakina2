#version 330 core
out vec3 TexCoords;

layout (location = 0) in vec3 a_Pos;

uniform mat4 a_mvp;

void main()
{
  TexCoords = a_Pos;
  vec4 pos = a_mvp * vec4(a_Pos, 1.0);
  gl_Position = pos.xyww;
}
