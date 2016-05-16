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



//GL Includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <math.h>


#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>


bool USE_OPENGL=false;

using namespace cv;
using namespace std;




//Terrible, Nasty Global Variables
//Needed GL Vars
GLuint rectifyShader, disparityShader, program;
vector<int> indices;

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
GLuint LoadShaders(const char * vertex_file, const char * fragment_file);
void GL_initialize();
vector<int> genIndices(int picWidth, int picHeight);
int calibrate();
void opengl_remap();
void Init_SBM();
void Init_SGBM();



int main(int argc, char *argv[]) {

	//init our opengl stuff.
//	GL_initialize();

	if(calibrate() != 0)//run the calibration to retrieve our matrices for transformation
	{
		return -1;
	}
	//now that we've generated the rectification transforms for the images, let's rectify them
	//Generate these first, as they remain constant as long as the cameras do.


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


	if(USE_OPENGL == true){
		//will need to read images in in a format opengl understands
		opengl_remap();
	}else{
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
	}

	return 0;

}



int calibrate(){
	//initialize the size of the board to 6x9
	Size boardSize(6, 9);

	//read in the image files
	string camera1image1fn = "res/left01.jpg";
	string camera1image2fn = "res/left02.jpg";
	string camera2image1fn = "res/right01.jpg";
	string camera2image2fn = "res/right02.jpg";

	/*
		string camera1image1fn = "res/myLeft01.jpg";
		string camera1image2fn = "res/myLeft01.jpg";
		string camera2image1fn = "res/myRight01.jpg";
		string camera2image2fn = "res/myRight01.jpg";
	 */


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
//	namedWindow("Image11", 1);
//	namedWindow("Image12", 1);
//	namedWindow("Image21", 1);
//	namedWindow("Image22", 1);


	//imshow("Image11", camera1image1);
	//imshow("Image12", camera1image2);
	//imshow("Image21", camera2image1);
	//imshow("Image22", camera2image2);


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


	//display the images with the corners drawn on them, this is just a sanity check
	//namedWindow("Corne11", 1);
	//namedWindow("Corne12", 1);
	//namedWindow("Corne21", 1);
	//namedWindow("Corne22", 1);


	//drawChessboardCorners(camera1image1, boardSize, camera1ImagePoints[0], true);
	//imshow("Corne11", camera1image1);

	//drawChessboardCorners(camera1image2, boardSize, camera1ImagePoints[1], true);
	//imshow("Corne12", camera1image2);

	//drawChessboardCorners(camera2image1, boardSize, camera2ImagePoints[0], true);
	//imshow("Corne21", camera2image1);

	//drawChessboardCorners(camera2image2, boardSize, camera2ImagePoints[1], true);
	//imshow("Corne22", camera2image2);


	//waitKey(0);
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

void GL_initialize() {
	//set up opengl window to render into

	rectifyShader = LoadShaders("res/rectify.vs", "res/rectify.fs");
	disparityShader = LoadShaders("res/disparity.vs", "res/disparity.fs");


}

void Init_SBM(){
	bm = StereoBM::create(64, 7); //create the StereoBM Object

	//bm->setROI1(); //usable area in rectified image
	//bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	//bm->setBlockSize(9); //block size to check
//	bm->setMinDisparity(-32);
	bm->setNumDisparities(128); //number of disparities
	//bm->setTextureThreshold(10);
//	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(16);
	bm->setSpeckleRange(0);
//	bm->setDisp12MaxDiff(1);

}
void Init_SGBM(){
	sgbm = StereoSGBM::create(0, 16, 3); //create the StereoBM Object


	sgbm->setPreFilterCap(63);
	sgbm->setBlockSize(3);
	sgbm->setP1(72);
	sgbm->setP2(256);
	sgbm->setMinDisparity(0);
	sgbm->setNumDisparities(192);
	sgbm->setUniquenessRatio(10);
	sgbm->setSpeckleRange(8);
	sgbm->setDisp12MaxDiff(1);


}
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
	vector<int> temp(0);

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
	// Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);
	//glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);


	glutSwapBuffers();

}

void opengl_remap(){


}

