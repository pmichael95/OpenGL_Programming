#version 330 core

in vec3 ourPosition;

out vec4 color;

void main()
{
    color = vec4(0.4f + (ourPosition.z), 0.7f + (ourPosition.z), 0.0f + (ourPosition.z), 1.0f);
} 