#include <cstdio>
#include <cstring>

//include the opencv libraries
#include <cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>


using namespace std;
using namespace cv;

int n_boards = 0;
int board_w;
int board_h;
int corner_count;
int successes = 0;

int main(int argc, const char** argv)
{
	char str[15], ext[15];

	int board_h = 0, board_w = 0;
	printf("Enter the name of the Images (String) without numeric denominator:  ");
	scanf("%s", str);
	printf("Enter the extension of the Images (String):  ");
	scanf("%s", ext);
	printf("Enter the total number of the File (should be integer) starting with index << 1 >> :: ");
	scanf("%d", &n_boards);
	printf("Enter the the board width (should be integer) :: ");
	scanf("%d", &board_w);
	printf("Enter the the board height (should be integer) :: ");
	scanf("%d", &board_h);



	char imgstr[total - 1][15];

	for (int i = 1; i <= total; i++)
	{
		sprintf(imgstr[i - 1], "%s%d", str, i);
		strcat(imgstr[i - 1], ".");
		strcat(imgstr[i - 1], ext);
	}


	int board_total = board_w * board_h;			//Total corners on the board
	CvSize board_sz = cvSize(board_w, board_h);

	CvMat* image_points = cvCreateMat(n_boards*board_total, 2, CV_32FC1);
	CvMat* object_points = cvCreateMat(n_boards*board_total, 3, CV_32FC1);
	CvMat* point_counts = cvCreateMat(n_boards, 1, CV_32SC1);
	CvMat* intrinsic_matrix = cvCreateMat(3, 3, CV_32FC1);
	CvMat* distortion_coeffs = cvCreateMat(4, 1, CV_32FC1);
	CvPoint2D32f* corners = new CvPoint2D32f[board_total];




	printf("Loading Images....... \n\n");
	IplImage* image;
	IplImage *gray_image;
	cvNamedWindow("Snapshot");

	char final_images[total - 1][15];
	int k = 0;
	for (int i = 0; i< total; i++)
	{
		image = cvLoadImage(imgstr[i], 1);
		if (image == NULL)
		{
			printf("unable to load the frame --> %s\n", imgstr[i]); exit(0);
		}
		gray_image = cvCreateImage(cvGetSize(image), 8, 1);

		//Find chessboard corners:
		int found = cvFindChessboardCorners(image, board_sz, corners, &corner_count, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
		//Convert from RGB to GRAY
		cvCvtColor(image, gray_image, CV_BGR2GRAY);

		//Get Subpixel accuracy on those corners
		cvFindCornerSubPix(gray_image, corners, corner_count, cvSize(11, 11), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

		//Draw the corners onto the chessboard
		cvDrawChessboardCorners(image, board_sz, corners, corner_count, found);

		// If we got a good board, add it to our data
		printf("%s is Loaded ..... \n", imgstr[i]);
		printf("Corner count for %s is << %d >> \n", imgstr[i], corner_count);

		if (corner_count == board_total)
		{
			step = successes*board_total;
			sprintf(final_images[k], "%s", imgstr[i]);
			k++;
			for (int i = step, j = 0; j<board_total; ++i, ++j)
			{
				CV_MAT_ELEM(*image_points, float, i, 0) = corners[j].x;
				CV_MAT_ELEM(*image_points, float, i, 1) = corners[j].y;
				CV_MAT_ELEM(*object_points, float, i, 0) = (float)j / board_w;
				CV_MAT_ELEM(*object_points, float, i, 1) = (float)(j%board_w);
				CV_MAT_ELEM(*object_points, float, i, 2) = 0.0f;
			}
			cvShowImage("Snapshot", image);
			cvWaitKey(3000);
			CV_MAT_ELEM(*point_counts, int, successes, 0) = board_total;
			successes++;
		}
		else {
			cvShowImage("Snapshot", image);
			cvWaitKey(3000);
		}

	}
	cvDestroyWindow("Snapshot");



	// Initialize the intrinsic matrix with both the two focal lengths in a ratio of 1.0

	CV_MAT_ELEM(*intrinsic_matrix, float, 0, 0) = 1.0f;
	CV_MAT_ELEM(*intrinsic_matrix, float, 1, 1) = 1.0f;

	//Calibrate the camera

	cvCalibrateCamera2(object_points2, image_points2, point_counts2, cvGetSize(image), intrinsic_matrix, distortion_coeffs, rot_vects, trans_vects, 0);

	//Save values to file
	printf(" *** Calibration Done!\n\n");
	printf("Calculted Camera Matrix is: \n\n");
	for (int i = 0; i<3; i++)
	{
		printf("%f %f %f\n", CV_MAT_ELEM(*intrinsic_matrix, float, i, 0), CV_MAT_ELEM(*intrinsic_matrix, float, i, 1), CV_MAT_ELEM(*intrinsic_matrix, float, i, 2));

	}
	printf("Storing Intrinsics.xml and Distortions.xml files...\n");
	cvSave("Intrinsics.xml", intrinsic_matrix);
	cvSave("Distortion.xml", distortion_coeffs);
	printf("Files saved.\n\n");

	printf("Starting corrected display (Only for Images in which all corners are detected)....\n\n");

	//Sample: load the matrices from the file
	CvMat *intrinsic = (CvMat*)cvLoad("Intrinsics.xml");
	CvMat *distortion = (CvMat*)cvLoad("Distortion.xml");

	// Build the undistort map used for all subsequent frames.

	IplImage* mapx = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	IplImage* mapy = cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 1);
	cvInitUndistortMap(intrinsic, distortion, mapx, mapy);

	// Run the camera to the screen, showing the raw and the undistorted image.

	cvNamedWindow("Rectified Image");
	cvNamedWindow("Input Images");

	char saveImage[k - 1][15];


	for (int i = 0; i<k; i++)
	{
		image = cvLoadImage(final_images[i], 1);
		IplImage *t = cvCloneImage(image);
		cvShowImage("Input Images", image);			// Show raw image
		cvRemap(t, image, mapx, mapy);			    // Undistort image
		cvReleaseImage(&t);
		cvShowImage("Rectified Image", image);			// Show corrected image
		sprintf(saveImage[i], "%s%d", str, i + 1);
		strcat(saveImage[i], "_rect.");
		strcat(saveImage[i], ext);
		cvSaveImage(saveImage[i], image);
		cvWaitKey(3000);

	}
	cvDestroyWindow("Rectified Image");
	cvDestroyWindow("Input Images");
}

