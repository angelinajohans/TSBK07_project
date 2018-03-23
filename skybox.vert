#version 150

in  vec3 in_Position;
in vec3 in_Normal;
in vec2 inTexCoord;
out vec3 ex_Normal;
out vec2 exTexCoord;

uniform mat4 projmatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

void main(void)
{
mat3 normalMatrix1 = mat3(mdlMatrix);
ex_Normal = normalMatrix1*in_Normal;
exTexCoord = inTexCoord;
gl_Position = projmatrix*camMatrix*mdlMatrix*vec4(in_Position, 1.0);
}
