
#define _CRT_SECURE_NO_WARNINGS 

#include "BMP.h"



using namespace cv;

//����·����ȡbmp�ļ�
Mat readBMP(const char* path) {
	BitMapFileHeader fileHeader;
	BitMapInfoHeader infoHeader;

	//���ļ�
	FILE* fp = fopen(path, "rb+");
	if (!fp) throw "File not found";

	//��ȡ�ļ�ͷ
	fread(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	if (fileHeader.bfType != 0x4D42) throw "invalid header";//��ͷ������BM�������Ǳ�׼�ĸ�ʽ

	//��ȡ��Ϣͷ
	fread(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);

	//�ƶ���ͼ�����ݵ���ʼ��
	fseek(fp, fileHeader.bfOffBits - sizeof(BitMapInfoHeader) - sizeof(BitMapFileHeader), SEEK_CUR);

	//��ӡ������Ϣ
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

	//0=û��ѹ������ѹ������ζ�ȡ��ʱû��ʵ��
	if (infoHeader.biCompression != 0) {
		printf("not implemented: compression %d", infoHeader.biCompression);
		throw "compression not implemented";
	}

	
	Mat res = Mat::zeros(abs(infoHeader.biHeight), infoHeader.biWidth, CV_8UC3);

	//bmp��ʽͼ��ÿ���ֽ����������4�ı�������0���롣����������㲹��֮��һ����ռ���ֽ���
	int rowByteNum = ((infoHeader.biWidth * infoHeader.biBitCount + 31) >> 5) << 2;
	//����һ��һ�е����ݵ�buffer
	unsigned char* rowBuffer = new unsigned char[rowByteNum];

	//���ղ�ͬ��λ���趨���ֽ�����ת������ɫ�Ĺ���
	std::function<Color(unsigned char*, int)> convertColor;
	switch (infoHeader.biBitCount) {
		//todo: other bitcount
	case 24://24λ BGR888
		convertColor = [](unsigned char* pixelStart, int bitOffset)->Color {
			return { pixelStart[0], pixelStart[1], pixelStart[2], 0 };
		};
		break;
	default:
		throw "bit count not implemented";
		break;
	}

	//��ȡ����
	for (int row = 0; row < abs(infoHeader.biHeight); row++) {
		//һ�ζ�ȡһ�е�buffer��
		fread(rowBuffer, sizeof(unsigned char), rowByteNum, fp);
		for (int col = 0; col < infoHeader.biWidth; col++) {
			// �ֽ�������bit������8
			int byteOffset = (col * infoHeader.biBitCount) >> 3;
			// bit����8ȡ�� ��24λͼ��������0
			int bitOffset = (col * infoHeader.biBitCount) & 7;
			// ת������ɫ
			Color c = convertColor(&rowBuffer[byteOffset], bitOffset);
			//����ɫ�Ž�Mat ����biHeight����������������������˳��
			if (infoHeader.biHeight < 0) {
				//biHeight�Ǹ��������������ϵ�������
				res.at<Vec3b>(row, col)[0] = c.b;
				res.at<Vec3b>(row, col)[1] = c.g;
				res.at<Vec3b>(row, col)[2] = c.r;
			}
			else {
				//biHeight�����������������µ�������
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

	//�ļ�ͷ
	fileHeader.bfType = 0x4D42;
	fileHeader.bfOffBits = 54;
	fileHeader.bfReserved = 0;
	fileHeader.bfSize = image.cols * image.rows * 3;

	//��Ϣͷ
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

	//���㲹�䵽4�ı���֮���ÿ���ֽ���
	int rowByteNum = ((image.rows * 3 + 3) >> 2) << 2;
	unsigned char* rowBuffer = new unsigned char[rowByteNum]();

	//���ļ���������д
	FILE* fp = fopen(path, "wb+");

	//д��ͷ
	fwrite(&fileHeader, sizeof(BitMapFileHeader), 1, fp);
	fwrite(&infoHeader, sizeof(BitMapInfoHeader), 1, fp);
	
	//д������
	for (int row = 0; row < image.rows; row++) {
		//ÿ����д��buffer��
		for (int col = 0; col < image.cols; col++) {
			auto color = image.at<Vec3b>(row, col);
			rowBuffer[(col) * 3 + 0] = color[0];
			rowBuffer[(col) * 3 + 1] = color[1];
			rowBuffer[(col) * 3 + 2] = color[2];
		}
		//bufferд���ļ�
		fwrite(rowBuffer, sizeof(unsigned char), rowByteNum, fp);
	}
	fflush(fp);
	fclose(fp);
}
