#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 outColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    outColor = vec3(0.5+(position.x*0.8), 0.7+(position.x*0.8), 0.9+(position.x*0.8));
}