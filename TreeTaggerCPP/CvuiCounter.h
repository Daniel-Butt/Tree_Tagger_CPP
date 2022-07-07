#pragma once
#include "CvuiControl.h"
#include <string>

using namespace cv;

class CvuiCounter : public CvuiControl{
public:
	CvuiCounter(Mat* window, int x, int y, double defaultValue, double step, std::function<void(CvuiObject*, double)> onChange);
	void render();
	int getType();
	void setFormat(const char* format);

	double value;
	double step;

private:
	double lastValue;
	const char* format;

};

