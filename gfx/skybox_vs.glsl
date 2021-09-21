#version 330 core
out vec3 TexCoords;

layout (location = 0) in vec3 aPos;

uniform mat4 a_mvp;

void main()
{
    TexCoords = aPos;
    vec4 pos = a_mvp * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
