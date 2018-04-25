#version 150

in  vec3 in_Position;
in vec2 inTexCoord;
out vec2 exTexCoord;
out float pos;

uniform mat4 projmatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;
uniform float sunposition;


void main(void)
{
pos = sunposition/1000;
exTexCoord = inTexCoord;
gl_Position = projmatrix*camMatrix*mdlMatrix*vec4(in_Position, 1.0);
}
