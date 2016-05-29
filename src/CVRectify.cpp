#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/ximgproc.hpp"



using namespace cv;
using namespace std;

Mat camera1image;
Mat camera2image;
Size imSize;
Mat rotationMatrices[2];
Mat projectionMatrices[2];


int main(){


	//open file and read in values
	ifstream infile;
	infile.open("res/CalibrationInfo.txt");

	double a, b, c;
	double arr[3][3];
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		arr[i][0] = a;
		arr[i][1] = b;
		arr[i][2] = c;
	}
	rotationMatrices[0] = Mat(3, 3, CV_64F, arr);

	for(int i = 0; i < 3; i ++){
			infile >> a >> b >> c;
			arr[i][0] = a;
			arr[i][1] = b;
			arr[i][2] = c;
	}
	rotationMatrices[1] = Mat(3, 3, CV_64F, arr);

	double d;
	double arr2[4][4];
	for(int i = 0; i < 4; i ++){
			infile >> a >> b >> c >> d;
			arr2[i][0] = a;
			arr2[i][1] = b;
			arr2[i][2] = c;
			arr2[i][3] = d;
	}
	projectionMatrices[0] = Mat(4, 4, CV_64F, arr2);

	for(int i = 0; i < 4; i ++){
			infile >> a >> b >> c >> d;
			arr2[i][0] = a;
			arr2[i][1] = b;
			arr2[i][2] = c;
			arr2[i][3] = d;
	}
	projectionMatrices[1] = Mat(4, 4, CV_64F, arr2);


	cout << rotationMatrices[0] << endl;
	cout << rotationMatrices[1] << endl;
	cout << projectionMatrices[0] << endl;
	cout << projectionMatrices[1] << endl;



	return 0;

	infile.close();


	//Mats used for remapping images to their rectified selves
	Mat map11, map12, map21, map22;
	initUndistortRectifyMap(cameraMatrices[0], distortionCoefficients[0], rotationMatrices[0], projectionMatrices[0], imSize, CV_16SC2, map11, map12);
	initUndistortRectifyMap(cameraMatrices[1], distortionCoefficients[1], rotationMatrices[1], projectionMatrices[1], imSize, CV_16SC2, map21, map22);

	//init the Stereo Block Matcher, needs only be done once
	Init_SBM();

	//from here we split, if we're using cpu, we use the remap function to remap the images.
	//if we're using opengl we jump to our opengl rectify function.

	Mat img1rectified, img2rectified, disp, dispVis;
	namedWindow("Disparity Map", 1);

	while(true){

		remap(camera1image, img1rectified, map11, map12, INTER_LINEAR);
		remap(camera2image, img2rectified, map21, map22, INTER_LINEAR);

		//Calc the Disparity map using Stereo BlockMatching
		bm->compute(img1rectified, img2rectified, disp);
		Mat dispVis;
		cv::ximgproc::getDisparityVis(disp, dispVis, 1.0);
		imshow("Disparity Map", dispVis);
		waitKey(0);
	}
}
