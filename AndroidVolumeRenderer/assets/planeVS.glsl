#version 310 es
precision highp float;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 fsTexCoord;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f); 
    fsTexCoord = texCoord;
}