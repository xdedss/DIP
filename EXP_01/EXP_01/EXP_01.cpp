

#include <iostream>

#include "BMP.h"

using namespace cv;

int main()
{

	//读bmp图像
	Mat img = readBMP("in.bmp");

	//画一个方形
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			img.at<Vec3b>(i, j) = { 255, 0, 255 };
		}
	}

	//写bmp图像
	writeBMP("out.bmp", img);

	//显示
	imshow("test", img);
	waitKey(0);

}
