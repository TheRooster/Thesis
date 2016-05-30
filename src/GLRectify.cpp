<<<<<<< HEAD
=======
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "bcm_host.h"
>>>>>>> branch 'master' of https://github.com/TheRooster/Thesis.git
#include <iostream>
#include <fstream>
#include <vector>

#include "Common/esUtil.h"

#define DEBUG 0








using namespace std;
using namespace glm;


unsigned int imHeight, imWidth;
//Needed GL Vars
mat3 camera1;
mat3 camera2;

mat3 rotation1;
mat3 rotation2;

mat3x4 projection1;
mat3x4 projection2;


typedef struct
{
	// Handle to a program object
	GLuint rectifyProgramObject;
	GLuint disparityProgramObject;


	// Attribute locations
	GLint  positionLoc;
	GLint  texCoordLoc;

	// Sampler location
	GLint samplerLoc;

	// Texture handle
	GLuint textureId;

} UserData;


//GLuint rectifyShader, disparityShader;
//GLuint framebuffers;
//GLuint leftImage, rightImage;
//GLuint leftImageRectified, rightImageRectified;

//std::vector<int> indices;
//std::vector<int> indices2;

//GLuint vao1;
//GLuint vao2;

//void GL_initialize(int* argc, char ** argv);
//GLuint LoadShaders(const char * vertex_file, const char * fragment_file);
//void init_VAO();
//vector<int> genIndices(int picWidth, int picHeight);
void Draw ( ESContext *esContext );
int Init ( ESContext *esContext );
GLuint CreateSimpleTexture2D( );
void ShutDown ( ESContext *esContext );

int main(int argc, char ** argv){


	//open file and read in values
	std::ifstream infile;
	infile.open("res/CalibrationInfo.txt");

	infile >> imHeight >> imWidth;


	double a, b, c;
	//camera matrices
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		camera1[i][0] = a;
		camera1[i][1] = b;
		camera1[i][2] = c;
	}

	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		camera2[i][0] = a;
		camera2[i][1] = b;
		camera2[i][2] = c;
	}

	//rotation matrices
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		rotation1[i][0] = a;
		rotation1[i][1] = b;
		rotation1[i][2] = c;
	}

	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		rotation2[i][0] = a;
		rotation2[i][1] = b;
		rotation2[i][2] = c;
	}

	//projection matrices
	double d;
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c >> d;
		projection1[i][0] = a;
		projection1[i][1] = b;
		projection1[i][2] = c;
		projection1[i][3] = d;
	}

	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c >> d;
		projection2[i][0] = a;
		projection2[i][1] = b;
		projection2[i][2] = c;
		projection2[i][3] = d;
	}

	infile.close();
#if DEBUG
	cout << glm::to_string(camera1) << endl;
	cout << glm::to_string(camera2) << endl;
	cout << glm::to_string(rotation1) << endl;
	cout << glm::to_string(rotation2) << endl;
	cout << glm::to_string(projection1) << endl;
	cout << glm::to_string(projection2) << endl;
#endif


	//done reading in needed vars, now lets init our OpenGl Stuff
	ESContext esContext;
	UserData  userData;

	esInitContext ( &esContext );
	esContext.userData = &userData;

	esCreateWindow ( &esContext, "Simple Texture 2D", imWidth, imHeight, ES_WINDOW_RGB );

	if ( !Init ( &esContext ) )
		return 0;

	esRegisterDrawFunc ( &esContext, Draw );

	esMainLoop ( &esContext );

	ShutDown ( &esContext );
}

GLuint CreateSimpleTexture2D( )
{
   // Texture object handle
   GLuint textureId;

   // 2x2 Image, 3 bytes per pixel (R, G, B)
   GLubyte pixels[4 * 3] =
   {
      255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
      255, 255,   0  // Yellow
   };

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load the texture
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

   return textureId;

}



void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );

   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

}

#if 0
void Display(void){
	return;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	UserData *userData = esContext->userData;

	GLfloat *vVertices = malloc(imHeight * imWidth * 2 * sizeof(GLfloat));
	GLushort *indices = genIndices(imWidth, imHeight);

	glViewport(0, 0, esContext->width, esContext.height);

	glUseProgram(userData -> rectifyProgramObject);

	GLint transformLoc = glGetUniformLocation(rectifyShader, "transformMatrix");
	GLint projectionLoc = glGetUniformLocation(rectifyShader, "projectionMatrix");

	//Left Image
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	// Clear all attached buffers
	glViewport(0, 0, imWidth, imHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the matrices
	glUniformMatrix4fv(transformLoc, 1,GL_FALSE, (GLfloat *)&rotation1[0]);
	glUniformMatrix4fv(projectionLoc, 1,GL_FALSE, (GLfloat *)&projection1[0]);

	//render left image here
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
	/*
	//Right Image
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
	glViewport(0, 0, imSize.width, imSize.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the matrices
	glUniformMatrix4fv(transformLoc, 1, rotationMatrices[1]);
	glUniformMatrix4fv(projectionLoc, 1, projectionMatrices[1]);

	//render right image here
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);



	/////////////////////////////////////////////////////
	// Bind to default framebuffer again draw the depth map
	// //////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Clear all relevant buffers
	glViewport(0, 0, imSize.width, imSize.height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Screen
	glUseProgram(disparityShader);





	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rightImage);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rightImage);

	 */

}
#endif

int Init ( ESContext *esContext )
{
	esContext->userData = malloc(sizeof(UserData));
	UserData *userData = esContext->userData;

	GLbyte vShaderStr[] =
			"uniform mat4 transformMatrix; \n"
			"uniform mat4 projectionMatrix;\n"
			"uniform mat4 cameraMatrix;    \n"
			"attribute vec2 a_Position;    \n"
			"attribute vec2 a_texCoord;    \n"
			"varying vec2 v_texCoord;      \n"
			"void main()                   \n"
			"{                             \n"
			"   gl_Position = vec4(a_Position, 1.0f, 1.0f) * transformMatrix * projectionMatrix; \n"
			"   v_texCoord = a_texCoord;   \n"
			"}                             \n";

	GLbyte fShaderStr[] =
			"precision mediump float;                            \n"
			"varying vec2 v_texCoord;                            \n"
			"uniform sampler2D s_texture;                        \n"
			"void main()                                         \n"
			"{                                                   \n"
			"  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
			"}                                                   \n";

	// Load the shaders and get a linked program object
	userData->rectifyProgramObject = esLoadProgram ( vShaderStr, fShaderStr );

	// Get the attribute locations
	userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
	userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );

	// Get the sampler location
	userData->samplerLoc = glGetUniformLocation ( userData->programObject, "texture" );

	// Load the texture
	userData->textureId = CreateSimpleTexture2D ();

	glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
	return GL_TRUE;





#if 0
	glutInit(argc, argv);
	glewExperimental = GL_TRUE;
	glewInit();
	//set up opengl window to render into
	glutInitWindowSize(imWidth, imHeight);
	glutCreateWindow("DepthMap");
	//glutFullScreen();
	//Gen the framebuffers
	//glGenFramebuffers(2, &framebuffers);

	//Gen and setup the Textures that we'll be rendering into
	//glGenTextures(1, &leftImage);
	//glGenTextures(1, &rightImage);

	//glBindTexture(GL_TEXTURE_2D, leftImageRectified);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	glBindTexture(GL_TEXTURE_2D, rightImageRectified);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	glBindTexture(GL_TEXTURE_2D, 0);

	//Set up our VAO's
	init_VAO();
#endif

	//Compile our shaders
	//rectifyShader = LoadShaders("res/rectify.vs", "res/rectify.fs");
	//disparityShader = LoadShaders("res/disparity.vs", "res/disparity.fs");
}

#if 0
GLuint LoadShaders(const char * vertex_file, const char * fragment_file) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {

		cerr << "Can not open " << vertex_file << ". No such file exists!" << endl;
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {

		cerr << "Can not open " << fragment_file << ". No such file exists!" << endl;
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	cout << "Compiling shader : " << vertex_file << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {

		GLchar* log = new GLchar[InfoLogLength + 1];

		glGetShaderInfoLog(VertexShaderID, InfoLogLength, &InfoLogLength, log);

		std::cerr << log << std::endl;
		delete[] log;
	}



	// Compile Fragment Shader

	cout << "Compiling shader : " << fragment_file << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);



	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		GLchar* log = new GLchar[InfoLogLength + 1];
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, log);
		std::cerr << log << std::endl;
		delete[] log;
	}



	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		GLchar* log = new GLchar[InfoLogLength + 1];
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, log);
		std::cerr << log << std::endl;
		delete[] log;
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void init_VAO(){

	//initialize the VAO
	glGenVertexArrays(1,&vao1);
	cout << "VAO Init done " << endl;
	glBindVertexArray(vao1);
	GLuint handle[3];
	GLuint handle2[3];
	glGenBuffers(3, handle);

	std::vector<GLuint> tempVertices;
	std::vector<GLfloat> tempUVs;
	indices = genIndices(imWidth, imHeight);

	cout << "indices: ";

	for(int i: indices)
	{
		cout << i << ' ';
	}
	cout << endl;

	for(int i = 0; i < imHeight; i ++){
		for(int j = 0; j < imWidth; j++){
			tempVertices.push_back(j);
			tempVertices.push_back(i);

			tempUVs.push_back(((GLfloat)imWidth)/j);
			tempUVs.push_back(((GLfloat)imHeight)/i);
		}
	}


	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, tempVertices.size() * sizeof(GLfloat), &tempVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // Vertex position


	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, tempUVs.size() * sizeof(GLfloat), &tempUVs[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // UV position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	glGenBuffers(3, handle2);

	indices2 = genIndices(1,1);
	cout << "indices: ";

	for(int i: indices2)
	{
		cout << i << ' ';
	}
	cout << endl;

	int tmp[] = {0,0,1,0,0,1,1,1};

	glBindBuffer(GL_ARRAY_BUFFER, handle2[0]);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &tmp[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // Vertex position


	glBindBuffer(GL_ARRAY_BUFFER, handle2[1]);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), &tmp[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // UV position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle2[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2.size() * sizeof(GLuint), &indices2[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}


vector<int> genIndices(int picWidth, int picHeight){
	vector<int> temp;

	for(int i = 0; i < picHeight; i ++){
		for(int j = 0; j < picWidth -1; j++){
			if(i %2 == 0){//even rows
				temp.push_back(i * picWidth + j);
				temp.push_back((i * picWidth) + j + picWidth);
				temp.push_back(i * picWidth + j + 1);
			}
			else{ //odd rows
				temp.push_back(i * picWidth + j);
				temp.push_back(i * picWidth + j + 1);
				temp.push_back((i-1) * picWidth + j);
			}
		}

	}
	return temp;
}

#endif

void ShutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->textureId );

   // Delete program object
   glDeleteProgram ( userData->programObject );

   free(esContext->userData);
}
