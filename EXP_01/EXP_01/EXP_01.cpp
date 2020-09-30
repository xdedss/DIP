

#include <iostream>

#include "BMP.h"

using namespace cv;

int main()
{

	auto img = readBMP("d:/2x1.bmp");

	writeBMP("d:/3.bmp", img);

	imshow("test", img);
	waitKey(0);

}
