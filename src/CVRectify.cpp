#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/ximgproc.hpp"

#include <iostream>
#include <fstream>


using namespace cv;
using namespace std;

Mat camera1image;
Mat camera2image;
Size imSize;
Mat cameraMatrices[2];
Mat rotationMatrices[2];
Mat projectionMatrices[2];
Mat distortionCoefficients[2];

Ptr<StereoBM> bm;
Ptr<StereoSGBM> sgbm;


void Init_SBM();
void Init_SGBM();

int main(){


	//open file and read in values
	std::ifstream infile;
	infile.open("res/CalibrationInfo.txt");
	int w, h;
	//image size
	infile >> w >> h ;
	imSize.height = h;
	imSize.width = w;

	double a, b, c;
	double arr[3][3];
	//camera matrices
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		arr[i][0] = a;
		arr[i][1] = b;
		arr[i][2] = c;
	}
	cameraMatrices[0] = Mat(3, 3, CV_64F, arr);

	double arr1[3][3];
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		arr1[i][0] = a;
		arr1[i][1] = b;
		arr1[i][2] = c;
	}
	cameraMatrices[1] = Mat(3, 3, CV_64F, arr1);

	//rotation matrices
	double arr2[3][3];
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		arr2[i][0] = a;
		arr2[i][1] = b;
		arr2[i][2] = c;
	}
	rotationMatrices[0] = Mat(3, 3, CV_64F, arr2);

	double arr3[3][3];
	for(int i = 0; i < 3; i ++){
		infile >> a >> b >> c;
		arr3[i][0] = a;
		arr3[i][1] = b;
		arr3[i][2] = c;
	}
	rotationMatrices[1] = Mat(3, 3, CV_64F, arr3);
	//projection matrices
	double d;
	double arr4[4][4];
	for(int i = 0; i < 4; i ++){
		infile >> a >> b >> c >> d;
		arr4[i][0] = a;
		arr4[i][1] = b;
		arr4[i][2] = c;
		arr4[i][3] = d;
	}
	projectionMatrices[0] = Mat(4, 4, CV_64F, arr4);

	double arr5[4][4];
	for(int i = 0; i < 4; i ++){
		infile >> a >> b >> c >> d;
		arr5[i][0] = a;
		arr5[i][1] = b;
		arr5[i][2] = c;
		arr5[i][3] = d;
	}
	projectionMatrices[1] = Mat(4, 4, CV_64F, arr5);

	//distortion coefficients
	double arr6[8];
	for(int i = 0; i < 8; i ++)
	{
		infile >> arr6[i];
	}

	distortionCoefficients[0] = Mat(8, 1, CV_64F, arr6);

	double arr7[8];
	for(int i = 0; i < 8; i ++)
	{
		infile >> arr7[i];
	}

	distortionCoefficients[1] = Mat(8, 1, CV_64F, arr7);


	cout << cameraMatrices[0] << endl;
	cout << cameraMatrices[1] << endl;
	cout << rotationMatrices[0] << endl;
	cout << rotationMatrices[1] << endl;
	cout << projectionMatrices[0] << endl;
	cout << projectionMatrices[1] << endl;
	cout << distortionCoefficients[0] << endl;
	cout << distortionCoefficients[1] << endl;
	infile.close();
	return 0;



//Mats used for remapping images to their rectified selves
	Mat map11, map12, map21, map22;
	initUndistortRectifyMap(cameraMatrices[0], distortionCoefficients[0], rotationMatrices[0], projectionMatrices[0], imSize, CV_16SC2, map11, map12);
	initUndistortRectifyMap(cameraMatrices[1], distortionCoefficients[1], rotationMatrices[1], projectionMatrices[1], imSize, CV_16SC2, map21, map22);

	//init the Stereo Block Matcher, needs only be done once
	Init_SBM();

	Mat img1rectified, img2rectified, disp, dispVis;
	namedWindow("Disparity Map", 1);

	//while(true){

		remap(camera1image, img1rectified, map11, map12, INTER_LINEAR);
		remap(camera2image, img2rectified, map21, map22, INTER_LINEAR);

		//Calc the Disparity map using Stereo BlockMatching
		bm->compute(img1rectified, img2rectified, disp);
		Mat dispVis;
		cv::ximgproc::getDisparityVis(disp, dispVis, 1.0);
		imshow("Disparity Map", dispVis);
		waitKey(0);
	//}
}

void Init_SBM(){
	bm = StereoBM::create(64, 11); //create the StereoBM Object

	//bm->setROI1(); //usable area in rectified image
	//bm->setROI2(roi2);
	bm->setPreFilterCap(31);
	//bm->setBlockSize(9); //block size to check
	//	bm->setMinDisparity(-32);
	bm->setNumDisparities(128); //number of disparities
	bm->setTextureThreshold(32);
	//	bm->setUniquenessRatio(15);
	bm->setSpeckleWindowSize(96);
	bm->setSpeckleRange(64);
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

