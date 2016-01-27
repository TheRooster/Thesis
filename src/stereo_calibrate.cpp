#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>



using namespace cv;
using namespace std;

vector<Point3f> Create3DChessboardCoordinates(Size boardSize, float squareSize);

int main(int argc, char *argv[]) {

	try {
		//initialize the size of the board to 6x9
		Size boardSize(6, 9);

		//read in the image files
		string camera1image1fn = "res/left01.jpg";
		string camera1image2fn = "res/left02.jpg";
		string camera2image1fn = "res/right01.jpg";
		string camera2image2fn = "res/right02.jpg";


		const float squareSize = 1.0f;

		//read the images into openCV matrices
		Mat camera1image1 = imread(camera1image1fn, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		Mat camera1image2 = imread(camera1image2fn, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		Mat camera2image1 = imread(camera2image1fn, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		Mat camera2image2 = imread(camera2image2fn, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);

		Size imSize = camera1image1.size();

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


		//display the images with the corners drawn on them, this is just a sanity check
		namedWindow("Corners11", 1);
		namedWindow("Corners12", 1);
		namedWindow("Corners21", 1);
		namedWindow("Corners22", 1);


		drawChessboardCorners(camera1image1, boardSize, camera1ImagePoints[0], true);
		imshow("Corners11", camera1image1);

		drawChessboardCorners(camera1image2, boardSize, camera1ImagePoints[1], true);
		imshow("Corners12", camera1image2);

		drawChessboardCorners(camera2image1, boardSize, camera2ImagePoints[0], true);
		imshow("Corners21", camera2image1);

		drawChessboardCorners(camera2image2, boardSize, camera2ImagePoints[1], true);
		imshow("Corners22", camera2image2);


		//initialize our fake 3D coordinate system, one for each set of images
		vector<vector<Point3f> > objectPoints(2);
		//both are identical, because we're using the same chessboard in each
		objectPoints[0] = Create3DChessboardCoordinates(boardSize, squareSize);
		objectPoints[1] = Create3DChessboardCoordinates(boardSize, squareSize);

		//init the initial camera matrices
		Mat cameraMatrices[2];
		cameraMatrices[0] = Mat::eye(3, 3, CV_64F); //3x3 identity matrix of 64bit floating point numbers(doubles)
		cameraMatrices[1] = Mat::eye(3, 3, CV_64F);


		//init the output matrices for the stereoCalibrate step
		Mat distortionCoefficients[2]; //array of matrices for the distortion coefficients, one per camera
		Mat rotationMatrix; //a rotation matrix from one camera to the other
		Mat translationVector; //a translation vector from one camera to the other
		Mat essentialMatrix;
		Mat fundamentalMatrix;

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



		Mat3f rotationMatrices[2];
		Matx34f projectionMatrices[2];
		Mat4f disparityToDepth;

		//now we have the right parameters to rectify the images
		stereoRectify(cameraMatrices[0], distortionCoefficients[0], cameraMatrices[1], distortionCoefficients[1],
			imSize, rotationMatrix, translationVector, rotationMatrices[0], rotationMatrices[1], projectionMatrices[0], projectionMatrices[1],
			disparityToDepth, 0, 0, cvSize(0, 0));

		//now that we've generated the rectification transforms for the images, let's rectify them
		Mat map11, map12, map21, map22;
		initUndistortRectifyMap(cameraMatrices[0], distortionCoefficients[0], rotationMatrices[0], projectionMatrices[0], imSize, CV_16SC2, map11, map12);
		initUndistortRectifyMap(cameraMatrices[1], distortionCoefficients[1], rotationMatrices[1], projectionMatrices[1], imSize, CV_16SC2, map21, map22);

		Mat img1rectified, img2rectified;
		remap(camera1image1, img1rectified, map11, map12, INTER_LINEAR);
		remap(camera2image1, img2rectified, map21, map22, INTER_LINEAR);

		//now we have rectified images in img1rectified and img2rectified
		//lets generate a disparity map using Stereo Block Matching
		Ptr<StereoBM> bm = StereoBM::create(16, 9); //create the StereoBM Object

		//init some parameters
		//bm->setROI1(); //usable area in rectified image
		//bm->setROI2(roi2);
		bm->setPreFilterCap(31);
		bm->setBlockSize(9); //block size to check
		bm->setMinDisparity(0);
		bm->setNumDisparities(16); //number of disparities
		bm->setTextureThreshold(10);
		bm->setUniquenessRatio(15);
		bm->setSpeckleWindowSize(100);
		bm->setSpeckleRange(32);
		bm->setDisp12MaxDiff(1);


		Mat disp, disp8;
		bm->compute(img1rectified, img2rectified, disp);

		//now we have a 16 bit signed single channel image, containing disparity values scaled by 16.
		disp.convertTo(disp8, CV_8U, 255 / 16 * 16.);

		//lets show the images
		namedWindow("left", 1);
		imshow("left", img1rectified);
		namedWindow("right", 1);
		imshow("right", img2rectified);
		namedWindow("disparity", 0);
		imshow("disparity", disp8);
		printf("press any key to continue...");
		fflush(stdout);
		waitKey();
		printf("\n");

	}
	catch(cv::Exception& e)
	{
		const char* err_msg = e.what();
		std::cout << "exception caught: " << err_msg << std::endl;
	}
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