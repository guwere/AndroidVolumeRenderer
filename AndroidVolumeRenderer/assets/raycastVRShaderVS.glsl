// #version 310 es
// precision highp float;
#version 330

layout (location = 0) in vec3 position;
uniform mat4 MVP;

out vec3 fsPosition;

void main()
{
	fsPosition = position;
	gl_Position = MVP * vec4(position, 1.0);
}