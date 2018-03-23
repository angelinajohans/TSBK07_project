#version 150

in  vec3 in_Position;
in vec3 in_Normal;
out vec3 ex_Normal;

uniform mat4 projmatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

void main(void)
{
  gl_Position = projmatrix*camMatrix*mdlMatrix*vec4(in_Position, 1.0);
}
