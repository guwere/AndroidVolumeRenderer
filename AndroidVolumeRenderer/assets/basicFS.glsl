#version 310 es
precision highp float;

in vec3 fsPosition;
out vec4 outcolor;
void main()
{
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	if(fsPosition.x < 0.0)
		color.x = 1.0;
	if(fsPosition.y < 0.0)
		color.y = 1.0;
	if(fsPosition.z < 0.0)
		color.z = 1.0;
	outcolor = color;
}