#version 310 es
precision highp float;
uniform mat4 MVP;

layout (location = 0) in vec3 position;

//out vec3 fsPosition;

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
}