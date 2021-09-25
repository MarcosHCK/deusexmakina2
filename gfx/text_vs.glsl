#version 330 core
layout (location = 0) in vec2 a_xy;
layout (location = 1) in vec2 a_uv;
out vec2 uv;

uniform mat4 a_mvp;

void main()
{
  gl_Position = vec4(a_xy, 0.0, 1.0);
  uv = a_uv;
}
