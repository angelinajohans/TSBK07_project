#version 150

out vec4 out_Color;
in vec3 ex_Normal;
in vec2 exTexCoord;
in vec3 ex_Surface;
in float pos;
in float sunposy;
in float sunposx;
uniform sampler2D texUnit;

uniform mat4 camMatrix;

void main(void)
{

		vec3 light = vec3(camMatrix* vec4(0.0f, sunposy, sunposx, 1.0)) - ex_Surface;
		light = normalize(light);

		float diffuse, shade;
		float specular = 0.0;
		diffuse = dot(normalize(ex_Normal), light);
		diffuse = max(0.0, diffuse);

		vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
		vec3 r = reflect(-light, normalize(ex_Normal));
		vec3 v = normalize(-ex_Surface); // View direction

		if(dot(light, ex_Normal) > 0.0)
		{
			specular = dot(r, v);
			specular = max(specular, 0.01);
			specular = pow(specular, 100);
		}

		shade = 0.7*diffuse + 0.3*specular;

		if(sunposy<0.2)
		{
			shade = 0.2;
		}

		out_Color =  out_Color + shade * vec4(lightColor, 1.0)*texture(texUnit, exTexCoord*100)*pos;

}
