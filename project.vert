#version 150

in  vec3 in_Position;
in vec3 in_Normal;
in vec2 inTexCoord;
out vec3 ex_Normal;
out vec2 exTexCoord;
out vec3 ex_Surface;
out float pos;
out float sunposy;
out float sunposx;

uniform mat4 projmatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;
uniform float sunposition;
uniform float sunpositionx;



void main(void)
{
  pos = sunposition/1000; //Normalize sun height
  if (pos<0.2)
  {
    pos = 0.2;
  }
  sunposy = sunposition;
  sunposx = sunpositionx;
  mat3 normalMatrix1 = mat3(mdlMatrix);
  ex_Normal = normalMatrix1*in_Normal;
  exTexCoord = inTexCoord;
  ex_Surface = vec3(camMatrix*mdlMatrix * vec4(in_Position, 1.0));
  gl_Position = projmatrix*camMatrix*mdlMatrix*vec4(in_Position, 1.0);
}
