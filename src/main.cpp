#include <cstdio>

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
}