#version 310 es
precision highp float;
uniform mat4 MVP;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec3 fsPosition;

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	fsPosition = position;
}