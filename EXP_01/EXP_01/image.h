#pragma once

const int FORMAT_RGB888 = 0;
const int FORMAT_GRAY = 1;

struct Color {
public:
	unsigned char b, g, r, a;
};

struct Point {
public:
	int x, y;
};

class Image {

public:

	int width;
	int height;
	int format;

	Color* arr;

	Image(int width, int height) {
		arr = new Color[width * height]();
		this->width = width;
		this->height = height;
	}

	Color& operator[] (Point p) {
		if (!CheckBorder(p.x, p.y)) throw "index out of range";
		return *(arr + p.y * width + p.x);
	}

private:

	bool CheckBorder(int x, int y) {
		return (x < width && x >= 0) && (y < height && y >= 0);
	}

};
