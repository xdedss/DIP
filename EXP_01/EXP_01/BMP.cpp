
#define _CRT_SECURE_NO_WARNINGS 

#include "BMP.h"



using namespace cv;

Mat readBMP(const char* path) {
	BitMapFileHeader fileHeader;
	BitMapInfoHeader infoHeader;

	FILE* fp = fopen(path, "rb+");
	if (!fp) throw "File not found";

	fread(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	if (fileHeader.bfType != 0x4D42) throw "invalid header";

	fread(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);

	//fseek(fp, fileHeader.bfOffBits - sizeof(BitMapInfoHeader) - sizeof(BitMapFileHeader), SEEK_CUR);

	printf("bfOffBits\t%d\n", fileHeader.bfOffBits);
	printf("biSize\t%d\n", infoHeader.biSize);
	printf("biWidth\t%d\n", infoHeader.biWidth);
	printf("biHeight\t%d\n", infoHeader.biHeight);
	printf("biPlanes\t%d\n", infoHeader.biPlanes);
	printf("biBitCount\t%d\n", infoHeader.biBitCount);
	printf("biCompression\t%d\n", infoHeader.biCompression);
	printf("biSizeImage\t%d\n", infoHeader.biSizeImage);
	printf("biXPixelsPerMeter\t%d\n", infoHeader.biXPelsPerMeter);
	printf("biYPixelsPerMeter\t%d\n", infoHeader.biYPelsPerMeter);
	printf("biClrUsed\t%d\n", infoHeader.biClrUsed);
	printf("biClrImportant\t%d\n", infoHeader.biClrImportant);

	if (infoHeader.biCompression != 0) {
		printf("not implemented: compression %d", infoHeader.biCompression);
		throw "compression not implemented";
	}

	//Image res = *new Image(infoHeader.biWidth, abs(infoHeader.biHeight));
	Mat res = Mat::zeros(abs(infoHeader.biHeight), infoHeader.biWidth, CV_8UC3);
	int rowByteNum = ((infoHeader.biWidth * infoHeader.biBitCount + 31) >> 5) << 2;
	unsigned char* rowBuffer = new unsigned char[rowByteNum];

	std::function<Color(unsigned char*, int)> convertColor;
	switch (infoHeader.biBitCount) {
		//todo: other bitcount
	case 24:
		convertColor = [](unsigned char* pixelStart, int bitOffset)->Color {
			return { pixelStart[0], pixelStart[1], pixelStart[2], 0 };
		};
		break;
	default:
		throw "bit count not implemented";
		break;
	}

	printf("rowbyte:%d, %d\n", rowByteNum, sizeof(unsigned char));

	for (int row = 0; row < abs(infoHeader.biHeight); row++) {
		printf("%d\n", fread(rowBuffer, sizeof(unsigned char), rowByteNum, fp));
		for (int col = 0; col < infoHeader.biWidth; col++) {
			int byteOffset = (col * infoHeader.biBitCount) >> 3;// Г§вд8
			int bitOffset = (col * infoHeader.biBitCount) & 7;// 8ШЁгр
			Color c = convertColor(&rowBuffer[byteOffset], bitOffset);
			if (infoHeader.biHeight < 0) {
				res.at<Vec3b>(row, col)[0] = c.b;
				res.at<Vec3b>(row, col)[1] = c.g;
				res.at<Vec3b>(row, col)[2] = c.r;
			}
			else {
				res.at<Vec3b>(infoHeader.biHeight - 1 - row, col)[0] = c.b;
				res.at<Vec3b>(infoHeader.biHeight - 1 - row, col)[1] = c.g;
				res.at<Vec3b>(infoHeader.biHeight - 1 - row, col)[2] = c.r;
			}
			//printf("(%d, %d) = %d, %d, %d, %d\n", col, row, c.r, c.g, c.b, c.a);
		}
	}

	fclose(fp);

	return res;

}

void writeBMP(const char* path, cv::Mat image) {
	BitMapFileHeader fileHeader;
	BitMapInfoHeader infoHeader;

	fileHeader.bfType = 0x4D42;
	fileHeader.bfOffBits = 54;
	fileHeader.bfReserved = 0;
	fileHeader.bfSize = image.cols * image.rows * 3;

	infoHeader.biSize = 40;
	infoHeader.biWidth = image.cols;
	infoHeader.biHeight = -image.rows;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = image.cols * image.rows * 3;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	int rowByteNum = ((image.rows * 3 + 3) >> 2) << 2;
	unsigned char* rowBuffer = new unsigned char[rowByteNum]();

	FILE* fp = fopen(path, "wb+");

	fwrite(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	fwrite(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);
	
	for (int row = 0; row < image.rows; row++) {
		for (int col = 0; col < image.cols; col++) {
			auto color = image.at<Vec3b>(row, col);
			rowBuffer[(col) * 3 + 0] = color[0];
			rowBuffer[(col) * 3 + 1] = color[1];
			rowBuffer[(col) * 3 + 2] = color[2];
		}
		fwrite(rowBuffer, sizeof(unsigned char), rowByteNum, fp);
	}
	fflush(fp);
	fclose(fp);
}
