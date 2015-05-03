#version 310 es
precision highp float;

layout (location = 0) in vec3 position;
uniform mat4 transformMat;

out vec2 fsTexCoord;

void main()
{
    gl_Position =  vec4(position.x, position.y, 0.0f, 1.0f); 
    fsTexCoord = vec2(position.x,position.y);
}