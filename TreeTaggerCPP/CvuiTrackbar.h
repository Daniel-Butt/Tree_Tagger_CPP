#pragma once
#include "CvuiControl.h"
class CvuiTrackbar : public CvuiControl{
public:
	CvuiTrackbar(Mat* window, int x, int y, int w, std::function<void(CvuiObject*, double)> onChange);
	void render();
	int getType();
	void setFormat(const char* format);
	void setWidth(int w);
	int getWidth();

	double value;
	double min;
	double max;
	int segments;
	unsigned int options;
	double discreteStep;

private:
	int width;
	const char* format;
};

