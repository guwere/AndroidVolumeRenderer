#version 310 es
precision highp float;
precision highp sampler3D;
//#version 330

in vec3 fsPosition;
//in vec4 fsTexCoord;

uniform sampler3D volume;
uniform sampler2D transferFunc;
uniform vec3 camPos;

uniform int maxRaySteps;
uniform float rayStepSize;
uniform float gradientStepSize;

uniform vec3 lightPosition;

uniform mat4 modelMatrix;

// uniform float clipPlaneDistance;
// uniform vec3 clipPlaneNormal;

out vec4 fragColor;

vec3 CalculateNormal(vec3 texCoord)
{
	vec3 sample1, sample2;

	sample1.x = texture(volume, texCoord - vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample2.x = texture(volume, texCoord + vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample1.y = texture(volume, texCoord - vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample2.y = texture(volume, texCoord + vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample1.z = texture(volume, texCoord - vec3(0.0f, 0.0f, gradientStepSize)).x;
	sample2.z = texture(volume, texCoord + vec3(0.0f, 0.0f, gradientStepSize)).x;

	return normalize(sample2 - sample1);
}


vec4 CalculateLighting(vec4 color, vec3 N)
{
	vec3 lightDirection = normalize(fsPosition - lightPosition);
	vec4 diffuseLight = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4 specularLight = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	vec3 L, H;

	L = normalize(lightDirection);
	H = normalize(L + normalize(-fsPosition));
	
	float diff = clamp(dot(N,L), 0.0f, 1.0f);
	float amb = 0.3f;
	vec4 spec = specularLight * pow (max(0.0f, dot(H, N)), 100.0f); 

	return ((color * diff) + spec + (color * amb));
}


void main()
{
	vec4 finalColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec3 position = fsPosition;
	vec3 texCoord;
	vec4 color;
	vec3 normal;

	vec3 direction = position - camPos;
	direction = normalize(direction);

	float absorption = 0.0f;
	float opacity;

	// if (clipPlaneDistance != 0.0f)
	// {
	// 	int steps = 0;
	// 	vec3 point = vec3(0.0f, 0.0f, 0.0f);
	// 	float dist;

	// 	if (camPos.z > 0.0f)
	// 	{
	// 		point.z = 1.0f;
	// 		dist = clipPlaneDistance;
	// 	}
	// 	else
	// 	{
	// 		point.z = -1.0f;
	// 		dist = 2.0f - clipPlaneDistance;
	// 	}

	// 	while ((abs(dot((point - position), clipPlaneNormal)) < dist) && (steps < maxRaySteps))
	// 	{
	// 		position += (direction * rayStepSize);
	// 		steps++;
	// 	}
	// }

	for(int i=0; i<maxRaySteps; i++)
	{
		color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		
		texCoord = (position + 1.0f) * 0.5f; 
		float index = texture(volume, texCoord).x;
		//float index = texture(volume, position).x;
		
		color = vec4(texture(transferFunc, vec2(index, 0.0)));
		opacity = color.w;

		//normal = CalculateNormal(position);
		//color = CalculateLighting(color, normal);

		if ((absorption + opacity) > 1.0f)
			finalColor += color*(1.0f - absorption);
		else
			finalColor += color*opacity;

		absorption += opacity;

		position = position + (direction * rayStepSize);

		if (abs(position.x) > 1.0f || abs(position.y) > 1.0f || abs(position.z) > 1.0f || absorption >= 1.0f)
		{
			if (absorption < 1.0f)
				finalColor += vec4(1.0f, 1.0f, 1.0f, 1.0f) * (1.0f - absorption);

			break;
		}
	}

	fragColor = finalColor;
}