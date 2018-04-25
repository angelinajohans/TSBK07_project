#ifdef __APPLE__
#include <OpenGL/gl3.h>
// Linking hint for Lightweight IDE
// uses framework Cocoa
#endif
#include "MicroGlut.h"
#include "GL_utilities.h"
#include <math.h>
#include "loadobj.h"
#include "LoadTGA.h"
#include "VectorUtils3.h"

Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	int triangleCount = (tex->width-1) * (tex->height-1) * 2;
	int x, z;

	GLfloat *vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	GLfloat *texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	GLuint *indexArray = malloc(sizeof(GLuint) * triangleCount*3);

	for (x = 0; x < tex->width; x++)
	{
		for (z = 0; z < tex->height; z++)
		{

			// Vertex array. You need to scale this properly
			vertexArray[(x + z * tex->width)*3 + 0] = x / 0.1;
			vertexArray[(x + z * tex->width)*3 + 1] = (cos(x)+sin(z*10));
			vertexArray[(x + z * tex->width)*3 + 2] = z / 0.1;
		}

	}

	for (x = 0; x < tex->width; x++)
	{
		for (z = 0; z < tex->height; z++)
		{
			// Normal vectors. You need to calculate these.
			vec3 n = {0,1,0};
			if ( (x-1) > 0 && (z-1) > 0 && (z+1) < tex->height && (x+1) < tex->width)
			{
				vec3 vertex1 = {vertexArray[(x + 1 + (z + 1) * tex->width)*3 + 0], vertexArray[(x + 1 + (z + 1) * tex->width)*3 + 1], vertexArray[(x + 1 + (z + 1) * tex->width)*3 + 2]};
				vec3 vertex2;
				vertex2 = (vec3){vertexArray[((x-1) + z * tex->width)*3 + 0], vertexArray[((x-1) + z * tex->width)*3 + 1], vertexArray[((x-1) + z * tex->width)*3 + 2]};
				vec3 vertex3;
				vertex3 = (vec3){vertexArray[(x + (z-1) * tex->width)*3 + 0], vertexArray[(x + (z-1) * tex->width)*3 + 1], vertexArray[(x + (z-1) * tex->width)*3 + 2]};
				vec3 vector1 = VectorSub(vertex2, vertex1);
				vec3 vector2 = VectorSub(vertex3, vertex1);
				n = CrossProduct(vector1, vector2);
				n = Normalize(n);

				if (n.y < 0)
				{
					n = ScalarMult(n, -1);
				}

			}

			normalArray[(x + z * tex->width)*3 + 0] = n.x;
			normalArray[(x + z * tex->width)*3 + 1] = n.y;
			normalArray[(x + z * tex->width)*3 + 2] = n.z;

			// Texture coordinates. You may want to scale them.
			texCoordArray[(x + z * tex->width)*2 + 0] = x/100; // (float)x / tex->width;
			texCoordArray[(x + z * tex->width)*2 + 1] = z/100; // (float)z / tex->height;
		}
	}
	for (x = 0; x < tex->width-1; x++)
	for (z = 0; z < tex->height-1; z++)
	{
		// Triangle 1
		indexArray[(x + z * (tex->width-1))*6 + 0] = x + z * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 1] = x + (z+1) * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 2] = x+1 + z * tex->width;
		// Triangle 2
		indexArray[(x + z * (tex->width-1))*6 + 3] = x+1 + z * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 4] = x + (z+1) * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 5] = x+1 + (z+1) * tex->width;
	}


	// End of terrain generation

	// Create Model and upload to GPU:

	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount*3);

		return model;
	}

// Globals
// Data would normally be read from files

#define near 1.0
#define far 1000.0
#define right 0.5
#define left -0.5
#define top 0.5
#define bottom -0.5

GLfloat projectionMatrix[] = {
	2.0f*near/(right-left), 0.0f, (right+left)/(right-left), 0.0f,
	0.0f, 2.0f*near/(top-bottom), (top+bottom)/(top-bottom), 0.0f,
	0.0f, 0.0f, -(far + near)/(far - near), -2*far*near/(far - near),
	0.0f, 0.0f, -1.0f, 0.0f};


	GLfloat a = 0.0;
	vec3 campos = {0, 0.5, 20};
	vec3 forward = {0,0,-4};
	vec3 up = {0,1,0};
	int way = 1;
	vec3 pos1 = {0,-11.5,0};
	vec3 pos2 = {50,-11.5,0};
	vec3 forwardboat = {0,0,1};
	vec3 forwardboat2 = {0,0,1};
	vec3 vel1, vel2;

	mat4 rot, trans, trans1, modelToWorldMatrix, camMatrix, rot1, rot2, modelToWorldMatrixsun, rotsun, transsun;

	Model *walls, *ground, *sky, *bunny, *sea, *sun;

	// Reference to shader program
	GLuint program,programsky, programsea, programsun, myTex, myTexmask, bottomTex;
	TextureData blacktex;

	// vertex array object
	unsigned int windmillVertexArrayObjID;

	void init(void)
	{
		// vertex buffer object, used for uploading the geometry
		unsigned int windmillVertexBufferObjID;
		unsigned int windmillIndexBufferObjID;
		unsigned int windmillNormalBufferObjID;
		unsigned int windmillTexCoordBufferObjID;

		walls = LoadModelPlus("OldBoat.obj");
		ScaleModel(walls, 0.5,0.5,0.5);
		ReloadModelData(walls);

		bunny = LoadModelPlus("OldBoat.obj");
		ScaleModel(bunny, 0.5,0.5,0.5);
		ReloadModelData(bunny);
		//sea = LoadModelPlus("sea.obj");
		LoadTGATextureData("black.tga", &blacktex);
		sun = LoadModelPlus("groundsphere.obj");
		ScaleModel(sun, 30,30,30);
		ReloadModelData(sun);
		ground = LoadModelPlus("ground.obj");
		sky = LoadModelPlus("skybox.obj");
		LoadTGATextureSimple("SkyBox512.tga", &myTexmask);
		LoadTGATextureSimple("conc.tga", &myTex);
		LoadTGATextureSimple("BlueTexturedPlastic.tga", &bottomTex);
		sea = GenerateTerrain(&blacktex);

		// GL inits
		glClearColor(1.0,0,0,0);
		glEnable(GL_DEPTH_TEST);
		printError("GL inits");

		// Load and compile shader
		program = loadShaders("project.vert", "project.frag");
		programsun = loadShaders("sunshader.vert", "sunshader.frag");
		programsky = loadShaders("skybox.vert", "skybox.frag");
		programsea = loadShaders("seashader.vert", "seashader.frag");
		printError("init shader");

		// Upload geometry to the GPU:

		// Allocate and activate Vertex Array Object
		glGenVertexArrays(1, &windmillVertexArrayObjID);
		glBindVertexArray(windmillVertexArrayObjID);

		glGenBuffers(1, &windmillVertexBufferObjID);
		glGenBuffers(1, &windmillIndexBufferObjID);
		glGenBuffers(1, &windmillNormalBufferObjID);
		glGenBuffers(1, &windmillTexCoordBufferObjID);

		// End of upload of geometry

		printError("init arrays");
	}  //Close void init

	void OnTimer(int value)
	{
		glutPostRedisplay();

		glutTimerFunc(20, &OnTimer, value);
	}

	float side = 0, straight = 0;

	void collision(vec3 *pos1, vec3 *pos2, GLfloat radius, vec3 vel1, vec3  vel2){
		float dx = pos1->x - pos2->x;
		float dy = pos1->y - pos2->y;
		float dz = pos1->z - pos2->z;

		float dist = sqrt(dx*dx+dy*dy+dz*dz);

		if (dist <= (radius) && fabsf(vel1.x-vel2.x)>fabsf(vel1.y-vel2.y))
		{
			way += 1;
		}
	}

	void display(void)
	{
		printError("pre display");

		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat t = (GLfloat)glutGet(GLUT_ELAPSED_TIME);

		glDisable(GL_DEPTH_TEST);

		if(glutKeyIsDown('a'))
			forward = MultMat3Vec3(mat4tomat3(Ry(0.02)), forward);
		if(glutKeyIsDown('d'))
			forward = MultMat3Vec3(mat4tomat3(Ry(-0.02)), forward);
		if(glutKeyIsDown('w'))
			campos = VectorAdd(campos, ScalarMult(forward, 0.5));
		if(glutKeyIsDown('s'))
			campos = VectorSub(campos, ScalarMult(forward, 0.5));
		if(glutKeyIsDown('q'))
		{
			vec3 side = CrossProduct(forward, SetVector(0,1,0));
			campos = VectorSub(campos, ScalarMult(side, 0.1));
		}
		if(glutKeyIsDown('e'))
		{
			vec3 side = CrossProduct(forward, SetVector(0,1,0));
			campos = VectorAdd(campos, ScalarMult(side, 0.1));
		}

		transsun = T(0,-1000,0);
		rotsun = Rx(t/2500);
		modelToWorldMatrixsun = Mult(rotsun, transsun);

		glUseProgram(programsky);
		glUniformMatrix4fv(glGetUniformLocation(programsky, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-2,-5,0);
		rot = Ry(0/300);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(programsky, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		camMatrix = lookAtv(campos, VectorAdd(campos, forward), up);
		a += 0.1;
		camMatrix = Mult(camMatrix, T(side,0,straight));
		camMatrix.m[3] = 0;
		camMatrix.m[7] = 0;
		camMatrix.m[11] = 0;
		glUniformMatrix4fv(glGetUniformLocation(programsky, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTexmask);
		glUniform1i(glGetUniformLocation(programsky, "texUnit"), 0);
		DrawModel(sky, programsky, "in_Position", NULL, "inTexCoord");

		glEnable(GL_DEPTH_TEST);

		glUseProgram(programsun);
		glUniformMatrix4fv(glGetUniformLocation(programsun, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		glUniformMatrix4fv(glGetUniformLocation(programsun, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrixsun.m);
		glUniformMatrix4fv(glGetUniformLocation(programsun, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glUniform1i(glGetUniformLocation(programsun, "texUnit"), 0);
		DrawModel(sun, programsun, "in_Position", NULL, "inTexCoord");

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-2,-15,0);
		rot = Ry(0/300);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		camMatrix = lookAtv(campos, VectorAdd(campos, forward), up);
		camMatrix = Mult(camMatrix, T(side,0,straight));
		glUniformMatrix4fv(glGetUniformLocation(program, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
		DrawModel(ground, program, "in_Position", "in_Normal", "inTexCoord");

		glUseProgram(programsea);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(glGetUniformLocation(programsea, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-800,-11,-800);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(programsea, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		glUniformMatrix4fv(glGetUniformLocation(programsea, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, bottomTex);
		glUniform1i(glGetUniformLocation(programsea, "texUnit"), 0); // Texture unit 0
		DrawModel(sea, programsea, "in_Position", "in_Normal", "inTexCoord");
		glDisable(GL_BLEND);

		if (way % 2 == 0)
		{
			float x = pos1.x;
			float z = pos1.z;
			forwardboat = MultMat3Vec3(mat4tomat3(Ry(0.02)), forwardboat);
			pos1 = VectorAdd(pos1, ScalarMult(forwardboat, 0.5));
			float dx1 = pos1.x - x;
			float dz1 = pos1.z - z;
			if (dz1 != 0)
			{
				rot1 = Ry(atan(dx1/dz1));
				if (dz1 < 0)
				{
					rot1 = Mult(rot1, Ry(acos(-1.0)));
				}
			}
			vel1 = SetVector(x, pos1.x, 1);
			x = pos2.x;
			z = pos2.z;
			forwardboat2 = MultMat3Vec3(mat4tomat3(Ry(0.08)), forwardboat2);
			pos2 = VectorAdd(pos2, ScalarMult(forwardboat2, 0.9));
			float dx2 = pos2.x - x;
			float dz2 = pos2.z - z;
			if (dz2 != 0)
			{
				rot2 = Ry(atan(dx2/dz2));
				if (dz2 < 0)
				{
					rot2 = Mult(rot2, Ry(acos(-1.0)));
				}
			}


		vel2 = SetVector(x, pos2.x, 1);
		}

		else
		{
			float x = pos1.x;
			float z = pos1.z;
			forwardboat = MultMat3Vec3(mat4tomat3(Ry(-0.02)), forwardboat);
			pos1 = VectorSub(pos1, ScalarMult(forwardboat, 0.5));
			float dx1 = pos1.x - x;
			float dz1 = pos1.z - z;
			if (dz1 != 0)
			{
				rot1 = Ry(atan(dx1/dz1));
				if (dz1 < 0)
				{
					rot1 = Mult(rot1, Ry(acos(-1.0)));
				}
			}
			vel1 = SetVector(x, pos1.x, 1);
			x = pos2.x;
			z = pos2.z;
			forwardboat2 = MultMat3Vec3(mat4tomat3(Ry(-0.08)), forwardboat2);
			pos2 = VectorSub(pos2, ScalarMult(forwardboat2, 0.9));
			float dx2 = pos2.x - x;
			float dz2 = pos2.z - z;
			if (dz2 != 0)
			{
				rot2 = Ry(atan(dx2/dz2));
				if (dz2 < 0)
				{
					rot2 = Mult(rot2, Ry(acos(-1.0)));
				}
			}

			vel2 = SetVector(x, pos2.x, 1);

		}

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		glUniformMatrix4fv(glGetUniformLocation(program, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		trans1 = T(1,-12, 1);
		//trans = T(pos1.x, pos1.y, pos1.z);
		rot1 = Mult(rot2, Ry(0));
		modelToWorldMatrix = Mult(trans1, rot1);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 0);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		DrawModel(walls, program, "in_Position", "in_Normal", "inTexCoord");

		trans = T(pos2.x, pos2.y, pos2.z);
		modelToWorldMatrix = Mult(trans, rot2);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		DrawModel(bunny, program, "in_Position", "in_Normal", "inTexCoord");

		glUniform1f(glGetUniformLocation(program, "sunposition"), modelToWorldMatrixsun.m[7]);
		glUniform1f(glGetUniformLocation(program, "sunpositionx"), modelToWorldMatrixsun.m[11]);

		glUseProgram(programsky);
		glUniform1f(glGetUniformLocation(programsky, "sunposition"), modelToWorldMatrixsun.m[7]);
		glUseProgram(programsea);
		glUniform1f(glGetUniformLocation(programsea, "sunposition"), modelToWorldMatrixsun.m[7]);
		glUniform1f(glGetUniformLocation(programsea, "sunpositionx"), modelToWorldMatrixsun.m[11]);

		printError("display");

		glutSwapBuffers();

		collision(&pos1, &pos2, 17, vel1, vel2);
	}


	int main(int argc, char *argv[])
	{
		glutInit(&argc, argv);
		glutInitContextVersion(3, 2);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); //Var ska den här ligga?
		glutCreateWindow ("Lab3");
		glutDisplayFunc(display);
		init ();
		glutTimerFunc(20, &OnTimer, 0);
		printf("Move camera with a,s,d and w\n");
		glutMainLoop();
		return 0;
	}
