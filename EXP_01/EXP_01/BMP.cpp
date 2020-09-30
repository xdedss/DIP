
#define _CRT_SECURE_NO_WARNINGS 

#include "BMP.h"



using namespace cv;

//根据路径读取bmp文件
Mat readBMP(const char* path) {
	BitMapFileHeader fileHeader;
	BitMapInfoHeader infoHeader;

	//打开文件
	FILE* fp = fopen(path, "rb+");
	if (!fp) throw "File not found";

	//读取文件头
	fread(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	if (fileHeader.bfType != 0x4D42) throw "invalid header";//开头必须是BM，否则不是标准的格式

	//读取信息头
	fread(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);

	//移动到图像数据的起始处
	fseek(fp, fileHeader.bfOffBits - sizeof(BitMapInfoHeader) - sizeof(BitMapFileHeader), SEEK_CUR);

	//打印各项信息
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

	//0=没有压缩，有压缩的如何读取暂时没有实现
	if (infoHeader.biCompression != 0) {
		printf("not implemented: compression %d", infoHeader.biCompression);
		throw "compression not implemented";
	}

	
	Mat res = Mat::zeros(abs(infoHeader.biHeight), infoHeader.biWidth, CV_8UC3);

	//bmp格式图像每行字节数如果不是4的倍数会用0补齐。所以这里计算补齐之后一行所占的字节数
	int rowByteNum = ((infoHeader.biWidth * infoHeader.biBitCount + 31) >> 5) << 2;
	//创建一个一行的数据的buffer
	unsigned char* rowBuffer = new unsigned char[rowByteNum];

	//按照不同的位数设定从字节数据转换到颜色的规则
	std::function<Color(unsigned char*, int)> convertColor;
	switch (infoHeader.biBitCount) {
		//todo: other bitcount
	case 24://24位 BGR888
		convertColor = [](unsigned char* pixelStart, int bitOffset)->Color {
			return { pixelStart[0], pixelStart[1], pixelStart[2], 0 };
		};
		break;
	default:
		throw "bit count not implemented";
		break;
	}

	//读取数据
	for (int row = 0; row < abs(infoHeader.biHeight); row++) {
		//一次读取一行到buffer里
		fread(rowBuffer, sizeof(unsigned char), rowByteNum, fp);
		for (int col = 0; col < infoHeader.biWidth; col++) {
			// 字节数等于bit数整除8
			int byteOffset = (col * infoHeader.biBitCount) >> 3;
			// bit数对8取余 在24位图像里总是0
			int bitOffset = (col * infoHeader.biBitCount) & 7;
			// 转换成颜色
			Color c = convertColor(&rowBuffer[byteOffset], bitOffset);
			//把颜色放进Mat 按照biHeight的正负号区分行数的正反顺序
			if (infoHeader.biHeight < 0) {
				//biHeight是负数代表行数从上到下排列
				res.at<Vec3b>(row, col)[0] = c.b;
				res.at<Vec3b>(row, col)[1] = c.g;
				res.at<Vec3b>(row, col)[2] = c.r;
			}
			else {
				//biHeight是正数代表行数从下到上排列
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

	//文件头
	fileHeader.bfType = 0x4D42;
	fileHeader.bfOffBits = 54;
	fileHeader.bfReserved = 0;
	fileHeader.bfSize = image.cols * image.rows * 3;

	//信息头
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

	//计算补充到4的倍数之后的每行字节数
	int rowByteNum = ((image.rows * 3 + 3) >> 2) << 2;
	unsigned char* rowBuffer = new unsigned char[rowByteNum]();

	//打开文件，二进制写
	FILE* fp = fopen(path, "wb+");

	//写入头
	fwrite(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	fwrite(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);
	
	//写入数据
	for (int row = 0; row < image.rows; row++) {
		//每行先写入buffer中
		for (int col = 0; col < image.cols; col++) {
			auto color = image.at<Vec3b>(row, col);
			rowBuffer[(col) * 3 + 0] = color[0];
			rowBuffer[(col) * 3 + 1] = color[1];
			rowBuffer[(col) * 3 + 2] = color[2];
		}
		//buffer写入文件
		fwrite(rowBuffer, sizeof(unsigned char), rowByteNum, fp);
	}
	fflush(fp);
	fclose(fp);
}
