#pragma once
#include "CvuiObject.h"

using namespace cv;

class CvuiRect : public CvuiObject{

public:
	CvuiRect(Mat* window, int x, int y, int w, int h, unsigned int borderColor, unsigned int fillColor);
	void render();
	int getType();
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);
	void setBorderColor(unsigned int color);
	void setFillColor(unsigned int color);
	unsigned int getBorderColor();
	unsigned int getFillColor();


private:
	int width;
	int height;
	unsigned int fillColor;
	unsigned int borderColor;
};

