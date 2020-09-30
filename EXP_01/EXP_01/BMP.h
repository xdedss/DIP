#pragma once

#include <functional>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "utils.h"

#pragma pack(2)
typedef struct
{
	unsigned short bfType;
	unsigned long bfSize;
	unsigned long bfReserved;
	unsigned long bfOffBits;
} BitMapFileHeader;

typedef struct
{
	unsigned long biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
} BitMapInfoHeader;


cv::Mat readBMP(const char* path);

void writeBMP(const char* path, cv::Mat image);
