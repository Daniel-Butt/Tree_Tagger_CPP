#pragma once
#include "CvuiControl.h"
#include <string>

using namespace cv;

class CvuiButton : public CvuiControl{

public:
	CvuiButton(Mat* window, int x, int y, int w, int h, std::string label, std::function<void(CvuiObject*, double)> onClick);
	CvuiButton(Mat* window, int x, int y, std::string label, std::function<void(CvuiObject*, double)> onClick);
	void render();
	int getType();
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);

	std::string label;


private:
	int width;
	int height;

};

