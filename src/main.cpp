#include <cstdio>
#include <cstring>

//include the opencv libraries
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>


using namespace std;
using namespace cv;


int main(int argc, const char** argv)
{
	
	Mat image;
	image = imread("res/left01.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		printf("Could not open or find the image\n");
		return -1;
	}

	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", image);                   // Show our image inside it.

	waitKey(0);                                          // Wait for a keystroke in the window
	return 0;
}

