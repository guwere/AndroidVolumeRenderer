#version 310 es
precision highp float;
precision highp sampler3D;
//#version 330

in vec4 fsPosition;
//in vec4 fsTexCoord;

uniform sampler3D volume;
uniform sampler2D transferFunc;

out vec4 fragColor;



void main()
{
	vec3 texCoord = (fsPosition.xyz + 1.0) * 0.5; //[-1,1] to [0,1]
	float index = texture(volume, texCoord).x;
	vec4 color = vec4(texture(transferFunc, vec2(index, 0.0)));

	fragColor = color;
}