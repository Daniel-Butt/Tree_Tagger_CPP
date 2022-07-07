#pragma once
#include "CvuiControl.h"
#include "CvuiRect.h"
#include "CvuiText.h"

using namespace std;

class CvuiColorSelection : public CvuiControl{
public:
	CvuiColorSelection(Mat* window, int x, int y, int w, int h, unsigned int color, std::function<void(CvuiObject*, double)> onClick);
	void render();
	int getType();
	unsigned int getColor();
	void setColor(unsigned int color);
	unsigned int getBorderColor();
	void setBorderColor(unsigned int color);

	int width;
	int height;
	int tolerance;

private:
	unsigned int color;
	unsigned int borderColor;
	string colorStr;
};

