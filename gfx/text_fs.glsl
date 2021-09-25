#version 330 core
out vec4 color;
in vec2 uv;

uniform sampler2D a_charmap;
uniform vec3 a_color;

void main()
{
  color = vec4(a_color, 1.0) * vec4(1.0, 1.0, 1.0, texture(a_charmap, uv).r);
}
