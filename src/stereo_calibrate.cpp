#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/ximgproc.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>




#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>




using namespace cv;
using namespace std;


#if 0

//Terrible, Nasty Global Variables
//Needed GL Vars
GLuint rectifyShader, disparityShader;
GLuint framebuffers[2];
GLuint leftImage, rightImage;
vector<int> indices;
vector<int> indices2;

GLuint vao;
#endif

//CV Vars
Mat camera1image1;
Mat camera1image2;
Mat camera2image1;
Mat camera2image2;
Size imSize;

Mat distortionCoefficients[2]; //array of matrices for the distortion coefficients, one per camera
Mat rotationMatrix; //a rotation matrix from one camera to the other
Mat translationVector; //a translation vector from one camera to the other
Mat essentialMatrix;
Mat fundamentalMatrix;

Mat cameraMatrices[2];
Mat rotationMatrices[2];
Mat projectionMatrices[2];
Mat disparityToDepth;


//Stereo BM object for creating disparity image
Ptr<StereoBM> bm;

Ptr<StereoSGBM> sgbm;

//Our output disparity maps
Mat disp, disp8, disparityVis;


vector<Point3f> Create3DChessboardCoordinates(Size boardSize, float squareSize);
//GLuint LoadShaders(const char * vertex_file, const char * fragment_file);
void GL_initialize();
vector<int> genIndices(int picWidth, int picHeight);
//void opengl_remap();




int main(int argc, char *argv[]) {

	//initialize the size of the board to 6x9
	Size boardSize(6, 9);

	//read in the image files
	string camera1image1fn = "res/left01.jpg";
	string camera1image2fn = "res/left02.jpg";
	string camera2image1fn = "res/right01.jpg";
	string camera2image2fn = "res/right02.jpg";




	const float squareSize = 1.0f;

	//read the images into openCV matrices
	camera1image1 = imread(camera1image1fn, CV_LOAD_IMAGE_GRAYSCALE);
	camera1image2 = imread(camera1image2fn, CV_LOAD_IMAGE_GRAYSCALE);
	camera2image1 = imread(camera2image1fn, CV_LOAD_IMAGE_GRAYSCALE);
	camera2image2 = imread(camera2image2fn, CV_LOAD_IMAGE_GRAYSCALE);

	imSize = camera1image1.size();

	//check that the sizes of the images are the same
	if (camera1image2.size() != imSize || camera1image1.size() != imSize || camera2image2.size() != imSize)
	{
		std::cerr << "All images must be the same size!" << std::endl;
		return -1;
	}


	//we know the images are the same size, now find the chessboard corners in the first image
	//an array of arrays of points for the left camera, one array per image
	vector<vector<Point2f> > camera1ImagePoints(2);
	bool found = findChessboardCorners(camera1image1, boardSize, camera1ImagePoints[0]);//check to see if we can find the chessboard in the first image
	if (!found)
	{
		cerr << "Corners not found in image 1,1" << endl;
		exit(-1);
	}

	found = findChessboardCorners(camera1image2, boardSize, camera1ImagePoints[1]);
	if (!found)
	{
		cerr << "Corners not found in image 1,2" << endl;
		exit(-1);
	}

	//now the same for the right camera
	vector<vector<Point2f> > camera2ImagePoints(2);
	found = findChessboardCorners(camera2image1, boardSize, camera2ImagePoints[0]);//check to see if we can find the chessboard in the first image
	if (!found)
	{
		cerr << "Corners not found in image 2,1" << endl;
		exit(-1);
	}

	found = findChessboardCorners(camera2image2, boardSize, camera2ImagePoints[1]);
	if (!found)
	{
		cerr << "Corners not found in image 2,2" << endl;
		exit(-1);
	}


	//initialize our fake 3D coordinate system, one for each set of images
	vector<vector<Point3f> > objectPoints(2);



	//both are identical, because we're using the same chessboard in each
	objectPoints[0] = Create3DChessboardCoordinates(boardSize, squareSize);
	objectPoints[1] = Create3DChessboardCoordinates(boardSize, squareSize);
	//init the initial camera matrices
	cameraMatrices[0] = Mat::eye(3, 3, CV_64F); //3x3 identity matrix of 64bit floating point numbers(doubles)
	cameraMatrices[1] = Mat::eye(3, 3, CV_64F);


	//init the output matrices for the stereoCalibrate step


	double error = stereoCalibrate(objectPoints, camera1ImagePoints, camera2ImagePoints,
			cameraMatrices[0], distortionCoefficients[0],
			cameraMatrices[1], distortionCoefficients[1],
			imSize, rotationMatrix, translationVector,
			essentialMatrix, fundamentalMatrix,
			CV_CALIB_FIX_ASPECT_RATIO +
			CV_CALIB_ZERO_TANGENT_DIST +
			CV_CALIB_SAME_FOCAL_LENGTH +
			CV_CALIB_RATIONAL_MODEL +
			CV_CALIB_FIX_K3 +
			CV_CALIB_FIX_K4 +
			CV_CALIB_FIX_K5, TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5)); //Termination Criteria (Why move this between 2.4 and 3.0?)




	//now we have the right parameters to rectify the images
	stereoRectify(cameraMatrices[0], distortionCoefficients[0], cameraMatrices[1], distortionCoefficients[1],
			imSize, rotationMatrix, translationVector, rotationMatrices[0], rotationMatrices[1], projectionMatrices[0], projectionMatrices[1],
			disparityToDepth, 0, 0, cvSize(0, 0));


	ofstream outfile;
	outfile.open("res/CalibrationInfo.txt");
	outfile<< imSize.height << ' ' << imSize.width << endl << endl;
	//camera matrices
	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 3; j ++){
			outfile<< cameraMatrices[0].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;
	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 3; j ++){
			outfile<< cameraMatrices[1].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;
	//rotation matrices
	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 3; j ++){
			outfile<< rotationMatrices[0].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;

	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 3; j ++){
			outfile<< rotationMatrices[1].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;

	//projection matrices
	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 4; j ++){
			outfile<< projectionMatrices[0].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;

	for(int i = 0; i < 3; i ++){
		for(int j = 0; j < 4; j ++){
			outfile<< projectionMatrices[1].at<double>(i,j) << ' ';
		}
		outfile << endl;
	}
	outfile << endl;

	//distortion coeffiicients
	for(int i = 0; i < 8; i ++){

		outfile<< distortionCoefficients[0].at<double>(i) << endl;
	}
	outfile << endl;

	for(int i = 0; i < 8; i ++){

		outfile<< distortionCoefficients[1].at<double>(i) << endl;
	}
	outfile << endl;


	outfile.close();


	/*

	//Mats used for remapping images to their rectified selves
	Mat map11, map12, map21, map22;
	initUndistortRectifyMap(cameraMatrices[0], distortionCoefficients[0], rotationMatrices[0], projectionMatrices[0], imSize, CV_16SC2, map11, map12);
	initUndistortRectifyMap(cameraMatrices[1], distortionCoefficients[1], rotationMatrices[1], projectionMatrices[1], imSize, CV_16SC2, map21, map22);

	//init the Stereo Block Matcher, needs only be done once
	Init_SBM();

	//from here we split, if we're using cpu, we use the remap function to remap the images.
	//if we're using opengl we jump to our opengl rectify function.

	Mat img1rectified, img2rectified;
	namedWindow("LeftImageRectified", 1);
	namedWindow("RightImageRectified", 1);
	namedWindow("Disparity Map", 1);


	remap(camera1image1, img1rectified, map11, map12, INTER_LINEAR);
	remap(camera2image1, img2rectified, map21, map22, INTER_LINEAR);

	//Calc the Disparity map using Stereo BlockMatching
	bm->compute(img1rectified, img2rectified, disp);
	Mat dispVis;
	cv::ximgproc::getDisparityVis(disp, dispVis, 1.0);
	//Show the Results
	imshow("LeftImageRectified", img1rectified);
	imshow("RightImageRectified", img2rectified);
	imshow("Disparity Map", dispVis);
	waitKey(0);





	/*
	//init stuff
	try{
		//Now init our opengl stuff, we need imSize to be initialized for this
		GL_initialize();
	}catch( const std::exception& e) { // caught by reference to base
		std::cout << " a standard exception was caught, with message '" << e.what() << "'\n";
	}
	//Mats used for remapping images to their rectified selves
		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(cameraMatrices[0], distortionCoefficients[0], rotationMatrices[0], projectionMatrices[0], imSize, CV_16SC2, map11, map12);
		initUndistortRectifyMap(cameraMatrices[1], distortionCoefficients[1], rotationMatrices[1], projectionMatrices[1], imSize, CV_16SC2, map21, map22);

		//init the Stereo Block Matcher, needs only be done once
		Init_SBM();

		//from here we split, if we're using cpu, we use the remap function to remap the images.
		//if we're using opengl we jump to our opengl rectify function.

		Mat img1rectified, img2rectified;
		namedWindow("LeftImageRectified", 1);
		namedWindow("RightImageRectified", 1);
		namedWindow("Disparity Map", 1);





		//Remap the images
	opengl_remap();
	//Read in from cameras eventually.
	remap(camera1image1, img1rectified, map11, map12, INTER_LINEAR);
	remap(camera2image1, img2rectified, map21, map22, INTER_LINEAR);

	//Calc the Disparity map using Stereo BlockMatching
	bm->compute(img1rectified, img2rectified, disp);
	Mat dispVis;
	cv::ximgproc::getDisparityVis(disp, dispVis, 1.0);
	//Show the Results
	imshow("LeftImageRectified", img1rectified);
	imshow("RightImageRectified", img2rectified);
	imshow("Disparity Map", dispVis);
	waitKey(0);
	 */
	return 0;

}


vector<Point3f> Create3DChessboardCoordinates(Size boardSize, float squareSize) {

	//create a vector of coordinates
	std::vector<Point3f> corners;

	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			//push the individual coordinates of the chessboard corners
			//its a 3D point, so we assume that all points lay on the 0 z plane
			corners.push_back(Point3f(float(j*squareSize),float(i*squareSize), 0));
		}
	}

	return corners;
}
#if 0
void GL_initialize() {
	//set up opengl window to render into
	glutInitWindowSize(imSize.width, imSize.height);
	glutCreateWindow("DepthMap");
	glutFullScreen();

	//Gen the framebuffers
	glGenFramebuffers(2, &framebuffers);

	//Gen and setup the Textures that we'll be rendering into
	glGenTextures(1, &leftImage);
	glGenTextures(1, &rightImage);

	glBindTexture(GL_TEXTURE_2D, leftImage);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imSize.width, imSize.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, rightImage);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imSize.width, imSize.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	//Set up our VAO's
	init_VAO();


	//Compile our shaders
	rectifyShader = LoadShaders("res/rectify.vs", "res/rectify.fs");
	disparityShader = LoadShaders("res/disparity.vs", "res/disparity.fs");
}
#endif



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

vector<int> genIndices(int picWidth, int picHeight){
	vector<int> temp = new vector<int>();

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



void Display(void){

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glUseProgram(rectifyShader);
	GLint transformLoc = glGetUniformLocation(rectifyShader, "transformMatrix");
	GLint projectionLoc = glGetUniformLocation(rectifyShader, "projectionMatrix");

	//Left Image
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	// Clear all attached buffers
	glViewport(0, 0, imSize.width, imSize.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the matrices
	glUniformMatrix4fv(transformLoc, 1, rotationMatrices[0]);
	glUniformMatrix4fv(projectionLoc, 1, projectionMatrices[0]);

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

void init_VAO(){
	//initialize the VAO
	glGenVertexArrays(2, &vao);

	glBindVertexArray(vao[0]);

	unsigned int handle[3];
	unsigned int handle2[3];

	glGenBuffers(3, &handle);

	std::vector<GLuint> tempVertices;
	std::vector<GLfloat> tempUVs;
	indices = genIndices(imSize.width, imSize.height);

	for(int i = 0; i < imSize.height; i ++){
		for(int j = 0; j < imSize.width; j++){
			tempVertices.push_back(j);
			tempVertices.push_back(i);

			tempUVs.push_back(((GLfloat)imSize.width)/j);
			tempUVs.push_back(((GLfloat)imSize.height)/i);
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

	glBindVertexArray(vao[1]);
	glGenBuffers(3, &handle2);

	indices2 = genIndices(1,1);
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

void opengl_remap(){
	//load the images into a texture

}
#endif
