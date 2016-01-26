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
	
	//read settings file
	Settings s;
	const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		printf("Could not open the configuration file: \"%s\"\n",inputSettingsFile);
		return -1;
	}
	fs["Settings"] >> s;
	fs.release();                                         // close Settings file

	if (!s.goodInput)
	{
		printf("Invalid input detected. Application stopping. \n");
		return -1;
	}

	//now we've got our settings loaded
	//load in the images
	for (int i = 0;; ++i)
	{
		Mat view;
		bool blinkOutput = false;

		view = s.nextImage();

		//-----  If no more image, or got enough, then stop calibration and show result -------------
		if (mode == CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames)
		{
			if (runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints))
				mode = CALIBRATED;
			else
				mode = DETECTION;
		}
		if (view.empty())          // If no more images then run calibration, save and stop loop.
		{
			if (imagePoints.size() > 0)
				runCalibrationAndSave(s, imageSize, cameraMatrix, distCoeffs, imagePoints);
			break;
			imageSize = view.size();  // Format input image.
			if (s.flipVertical)    flip(view, view, 0);
		}
}

