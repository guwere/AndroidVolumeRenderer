#version 310 es
precision highp float;

layout (location = 0) in vec3 position;
uniform mat4 transformMat;

out vec2 fsTexCoord;

void main()
{
    gl_Position =   transformMat * vec4(position, 1.0f); 
    fsTexCoord = (vec2(position.x,position.y) + vec2(1,1)) * 0.5;
}


// #version 310 es
// precision highp float;

// layout (location = 0) in vec3 position;
// uniform mat4 transformMat;

// out vec2 fsTexCoord;

// void main()
// {
//     gl_Position =  vec4(position.x, position.y, 0.0f, 1.0f); 
//     fsTexCoord = vec2(position.x,position.y);
// }