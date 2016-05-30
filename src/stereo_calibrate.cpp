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

