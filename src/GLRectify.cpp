#include <iostream>
#include <fstream>
#include <vector>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include "Common/esUtil.h"
#include "glm/glm.hpp"

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

GLfloat * vertices;
GLshort * colors;
GLuint * indices;
int indicesCount = 0;


//IL stuff
ILuint ImgId;

typedef struct
{
	// Handle to a program object
	GLuint rectifyProgramObject;
	GLuint disparityProgramObject;


	// Attribute locations
	GLint  positionLoc;
	GLint  colorLoc;

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
GLfloat * init_VertexInfo();
GLshort * init_VertexColors(char * filename);
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

void Draw ( ESContext *esContext )
{
   UserData *userData =(UserData *)(esContext->userData);

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->rectifyProgramObject );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 4, GL_FLOAT,
                           GL_TRUE, 1, vertices);
   // Load the texture coordinate
   glVertexAttribPointer ( userData->colorLoc, 4, GL_SHORT,
                         GL_FALSE, 1, colors);

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->colorLoc );


   glDrawElements ( GL_TRIANGLES, indicesCount, GL_UNSIGNED_SHORT, indices );

}



int Init ( ESContext *esContext )
{
	//init the img data
	ilGenImages(1, &ImgId);


	esContext->userData = malloc(sizeof(UserData));
	UserData *userData = (UserData *)(esContext->userData);

	GLbyte vShaderStr[] =
			"uniform mat4 transformMatrix; \n"
			"uniform mat4 projectionMatrix;\n"
			"uniform mat4 cameraMatrix;    \n"
			"attribute vec4 a_Position;    \n"
			"attribute vec4 a_Color;    \n"
			"varying vec4 v_Color;      \n"
			"void main()                   \n"
			"{                             \n"
			"   gl_Position = a_Position;  \n"
			"   v_Color = a_Color;   \n"
			"}                             \n";

	GLbyte fShaderStr[] =
			"precision mediump float;                            \n"
			"varying vec4 v_Color;                               \n"
			"void main()                                         \n"
			"{                                                   \n"
			"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);        \n"
			"}                                                   \n";

	// Load the shaders and get a linked program object
	userData->rectifyProgramObject = esLoadProgram ((char *)vShaderStr, (char *)fShaderStr );

	// Get the attribute locations
	userData->positionLoc = glGetAttribLocation ( userData->rectifyProgramObject, "a_Position" );
	userData->colorLoc = glGetAttribLocation ( userData->rectifyProgramObject, "a_Color" );

	glClearColor ( 1.0f, 1.0f, 1.0f, 1.0f );

	vertices = init_VertexInfo();
	colors = init_VertexColors("res/left01.jpg");
	return GL_TRUE;
}


GLfloat * init_VertexInfo(){

	GLfloat * tmpVertexInfo = (GLfloat *)malloc(imWidth * imHeight * 4 * sizeof(GLfloat));//init memory for positions


	//indices = genIndices(imWidth, imHeight);

	for(int i = 0; i < imHeight; i ++){
		for(int j = 0; j < imWidth * 4; j+=4){
			int loc = (i*imWidth*4) + j;
			tmpVertexInfo[loc] = (GLfloat)j/4;
			tmpVertexInfo[loc + 1] = (GLfloat)i;
			tmpVertexInfo[loc + 2] = 1.0f;
			tmpVertexInfo[loc + 3] = 1.0f;
		}

	}
	return tmpVertexInfo;
}


GLshort * init_VertexColors(char * filename){
	cout << "Loading Image" << endl;

	GLshort *tmp = (GLshort *)malloc(imWidth * imHeight * 4 * sizeof(GLshort));
	for(int i = 0; i < imWidth * imHeight * 4; i ++)
	{
		if(i % 2 == 0){
			tmp[i] = 255;
		}
		else{
			tmp[i] = 0;
		}
	}

	return tmp;
	//ilBindImage(ImgId);
	//ilLoadImage(filename);

	//return (GLshort *) ilGetData();
}

GLuint * genIndices(int picWidth, int picHeight){
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
	indicesCount = temp.size();

	GLuint * indices = (GLint *)malloc(temp.size() * sizeof(GLint));
	for(int i = 0; i < temp.size; i ++){
		indices[i] = temp[i];
	}
	return indices;
}

void ShutDown ( ESContext *esContext )
{
   UserData *userData =(UserData *)(esContext->userData);

   // Delete texture object
   glDeleteTextures ( 1, &userData->textureId );

   // Delete program object
   glDeleteProgram ( userData->rectifyProgramObject );

   free(esContext->userData);
}
