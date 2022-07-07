#pragma once
#include "CvuiObject.h"
#include <string>

using namespace cv;

class CvuiText : public CvuiObject{
public:
	CvuiText(Mat* window, int x, int y, std::string label);
	CvuiText(Mat* window, int x, int y, std::string label, double fontScale);
	void render(); 
	int getType(); 
	void setColor(unsigned int color);
	void setFontScale(double fontScale);
	double getFontScale();

	std::string label;
	
private:
	double fontScale;
	unsigned int color;

};

