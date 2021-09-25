#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube a_skybox;

void main()
{    
  FragColor = texture(a_skybox, TexCoords);
}
