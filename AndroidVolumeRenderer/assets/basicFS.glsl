#version 310 es
precision highp float;

in vec3 fsPosition;
uniform vec4 color;
out vec4 outcolor;
void main()
{
	if(color.w == 0.0)
	{
		// vec4 finalColor = vec4(0.0,0.0,0.0,1.0);
		// if(fsPosition.x < 0.0)
		// 	finalColor.x = 1.0;
		// if(fsPosition.y < 0.0)
		// 	finalColor.y = 1.0;
		// if(fsPosition.z < 0.0)
		// 	finalColor.z = 1.0;
		// outcolor = finalColor;
		outcolor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		outcolor = color;
	}
}