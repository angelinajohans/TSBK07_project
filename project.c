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


Point3D lightSourcesColorsArr[] = { {1.0f, 0.0f, 0.0f}, // Red light

                                 {0.0f, 1.0f, 0.0f}, // Green light

                                 {0.0f, 0.0f, 1.0f}, // Blue light

                                 {1.0f, 1.0f, 1.0f} }; // White light

GLint isDirectional[] = {0,0,1,1};


Point3D lightSourcesDirectionsPositions[] = { {10.0f, 5.0f, 0.0f}, // Red light, positional

                                       {0.0f, 5.0f, 10.0f}, // Green light, positional

                                       {-1.0f, 0.0f, 0.0f}, // Blue light along X

                                       {0.0f, 0.0f, -1.0f} }; // White light along Z

GLfloat specularExponent[] = {100.0, 200.0, 60.0, 50.0, 300.0, 150.0};

	mat4 rot, trans, trans1, modelToWorldMatrix, camMatrix;
	vec3 pos[3];

	Model *walls, *ground, *sky, *bunny, *sea;

	int currentModelIndex = 0;
	GLfloat xpos;

	// Reference to shader program
	GLuint program,programsky, programsea, myTex, myTexmask, myTexx, bunnyTex;

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
		sea = LoadModelPlus("sea.obj");
		ground = LoadModelPlus("ground.obj");
		sky = LoadModelPlus("skybox.obj");
		LoadTGATextureSimple("SkyBox512.tga", &myTexmask);
		LoadTGATextureSimple("conc.tga", &myTex);
		LoadTGATextureSimple("grass.tga", &myTexx);
		LoadTGATextureSimple("maskros512.tga", &bunnyTex);

		dumpInfo();

		// GL inits
		glClearColor(1.0,0,0,0);
		glEnable(GL_DEPTH_TEST);
		printError("GL inits");

		// Load and compile shader
		program = loadShaders("project.vert", "project.frag");
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

		printError("pre bunny");
		// VBO for vertex data
		glBindBuffer(GL_ARRAY_BUFFER, windmillVertexBufferObjID);
		glBufferData(GL_ARRAY_BUFFER, walls->numVertices*3*sizeof(GLfloat), walls->vertexArray, GL_STATIC_DRAW);
		glVertexAttribPointer(glGetAttribLocation(program, "in_Position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(glGetAttribLocation(program, "in_Position"));

		// VBO for normal data
		glBindBuffer(GL_ARRAY_BUFFER, windmillNormalBufferObjID);
		glBufferData(GL_ARRAY_BUFFER, walls->numVertices*3*sizeof(GLfloat), walls->normalArray, GL_STATIC_DRAW);
		glVertexAttribPointer(glGetAttribLocation(program, "in_Normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(glGetAttribLocation(program, "in_Normal"));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, windmillIndexBufferObjID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, walls->numIndices*sizeof(GLuint), walls->indexArray, GL_STATIC_DRAW);

		if (walls->texCoordArray != NULL)

		{

			glBindBuffer(GL_ARRAY_BUFFER, windmillTexCoordBufferObjID);

			glBufferData(GL_ARRAY_BUFFER, walls->numVertices*2*sizeof(GLfloat), walls->texCoordArray, GL_STATIC_DRAW);

			glVertexAttribPointer(glGetAttribLocation(program, "inTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(glGetAttribLocation(program, "inTexCoord"));

		}

		// End of upload of geometry

		printError("init arrays");
	}  //Close void init

	void OnTimer(int value)
	{
		glutPostRedisplay();

		glutTimerFunc(20, &OnTimer, value);
	}

	float angle = 0, height = 0, side = 0, straight = 0;

	int prevx = 0, prevy = 0;


	void collision(vec3 *pos1, vec3 *pos2, GLfloat radius){
		float dx = pos1->x - pos2->x;
		float dy = pos1->y - pos2->y;
		float dz = pos1->z - pos2->z;

		float dist = sqrt(dx*dx+dy*dy+dz*dz);

		if (dist <= (radius*2)){
			printf("Hello");
		}
		else{
	
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


		glUseProgram(programsky);
		glUniformMatrix4fv(glGetUniformLocation(programsky, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-2,-5,0);
		rot = Ry(0/300);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(programsky, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		camMatrix = lookAtv(campos, VectorAdd(campos, forward), up);
		a += 0.1;
		//camMatrix = lookAt(25*sin(angle), -height, 25*cos(angle), 0, height,0, 0,1,0);
	 	camMatrix = Mult(camMatrix, T(side,0,straight));
		camMatrix.m[3] = 0;
		camMatrix.m[7] = 0;
		camMatrix.m[11] = 0;
		glUniformMatrix4fv(glGetUniformLocation(programsky, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTexmask);
		glUniform1i(glGetUniformLocation(programsky, "texUnit"), 0); // Texture unit 0
		DrawModel(sky, programsky, "in_Position", "in_Normal", "inTexCoord");

		glEnable(GL_DEPTH_TEST);

		/*glUseProgram(programsea);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(glGetUniformLocation(programsea, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-2,-11,0);
		rot = Ry(0/300);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(programsea, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		camMatrix = lookAtv(campos, VectorAdd(campos, forward), up);
	 	camMatrix = Mult(camMatrix, T(side,0,straight));
		glUniformMatrix4fv(glGetUniformLocation(programsky, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glUniform1i(glGetUniformLocation(programsea, "texUnit"), 0); // Texture unit 0
		DrawModel(sea, programsea, "in_Position", "in_Normal", NULL);
		glDisable(GL_BLEND);*/


		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "projmatrix"), 1, GL_TRUE, projectionMatrix);
		trans = T(-2,-11,0);
		rot = Ry(0/300);
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		camMatrix = lookAtv(campos, VectorAdd(campos, forward), up);
	 	camMatrix = Mult(camMatrix, T(side,0,straight));
		glUniformMatrix4fv(glGetUniformLocation(programsky, "camMatrix"), 1, GL_TRUE, camMatrix.m);
		glBindTexture(GL_TEXTURE_2D, myTex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 0); // Texture unit 0
		DrawModel(ground, program, "in_Position", "in_Normal", "inTexCoord");


		glBindTexture(GL_TEXTURE_2D, bunnyTex);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 0);


		pos[0] = SetVector(50*cos(t/600),-8, 50*sin(t/600));
		trans1 = T(pos[0].x, pos[0].y, pos[0].z);
		rot = Ry(-t/600);
		modelToWorldMatrix = Mult(trans1, rot);
		glBindTexture(GL_TEXTURE_2D, myTexx);
		glUniform1i(glGetUniformLocation(program, "texUnit"), 0); // Texture unit 0
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		DrawModel(walls, program, "in_Position", "in_Normal", NULL);

		modelToWorldMatrix = IdentityMatrix();
		currentModelIndex = 0;
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		pos[1] = SetVector(50*cos(t/600),-8, -50*sin(t/600));
		trans = T(pos[1].x, pos[1].y, pos[1].z);
		rot = Mult(Ry(t/600), Ry(3.1415));
		modelToWorldMatrix = Mult(trans, rot);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, modelToWorldMatrix.m);
		DrawModel(bunny, program, "in_Position", "in_Normal", NULL);
		//Specular
		glUniform3fv(glGetUniformLocation(program, "lightSourcesDirPosArr"), 4, &lightSourcesDirectionsPositions[0].x);
		glUniform3fv(glGetUniformLocation(program, "lightSourcesColorArr"), 4, &lightSourcesColorsArr[0].x);
		glUniform1f(glGetUniformLocation(program, "specularExponent"), specularExponent[0]);
		glUniform1iv(glGetUniformLocation(program, "isDirectional"), 4, isDirectional);

		printError("display");

		glutSwapBuffers();

		collision(&pos[0], &pos[1], 17);
	}


	int main(int argc, char *argv[])
	{
		glutInit(&argc, argv);
		glutInitContextVersion(3, 2);
		glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); //Var ska den här ligga?
		glutCreateWindow ("Lab3");
		glutDisplayFunc(display);
		//glutKeyboardFunc(keyboard);
		init ();
		glutTimerFunc(20, &OnTimer, 0);
		printf("Change object with 't' or menu\n");
		printf("Rotate camera with '.' and ','\n");
		printf("Change camera height with '+', '-'\n");
		printf("Move camera with a,s,d and w\n");
		glutMainLoop();
		return 0;
	}
