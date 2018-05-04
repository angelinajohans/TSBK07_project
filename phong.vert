#version 150

in  vec3 inPosition;
in  vec3 inNormal;
//in vec3 inTexCoord;
out vec3 exNormal; // Phong
out vec3 exSurface; // Phong (specular)
//out vec3 outTexcoord;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 camMatrix;

uniform int edge;
uniform int count;

void main(void)
{
//outTexcoord = inTexCoord;
	//exNormal = inverse(transpose(mat3(modelviewMatrix))) * inNormal; // Phong, "fake" normal transformation

	//exSurface = vec3(modelviewMatrix * vec4(inPosition, 1.0)); // Don't include projection here - we only want to go to view coordinates

	//int row = gl_InstanceID / edge;
	vec3 p = vec3((gl_InstanceID-edge/2) % edge,0 ,(gl_InstanceID-edge/2) / edge );
	vec3 d = p - vec3(0,-10,0);
	vec3 n = normalize(d);
	vec3 r = n*5;
	vec4 offs = vec4(r,0);
	//vec4 rr = vec4(r.x / edge, r.y / edge, r.z, 0);
	//vec3 offs = r;


	gl_Position = projectionMatrix * camMatrix* modelviewMatrix  * (vec4(inPosition, 1.0)+  offs *100) ; // This should include projection
}
