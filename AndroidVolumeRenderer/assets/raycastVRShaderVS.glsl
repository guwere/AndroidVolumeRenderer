#version 310 es
precision highp float;
//#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
uniform mat4 MVP;

out vec3 fsPosition;
out vec2 fsTexCoord;
void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	fsPosition = position;
	fsTexCoord = texCoord;
}