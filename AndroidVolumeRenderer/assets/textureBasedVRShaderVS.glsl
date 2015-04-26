#version 310 es
precision highp float;
//#version 330

layout (location = 0) in vec3 position;
uniform mat4 projMatrix;
uniform mat4 inverseMV;

out vec4 fsPosition;
void main()
{
	gl_Position = projMatrix * vec4(position, 1.0); // position already in view space 
	fsPosition = inverseMV * vec4(position, 1.0); // get the position back into model space to correspond to the texture coordinate space
}