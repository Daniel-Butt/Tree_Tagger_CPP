#pragma once
#include "CvuiControl.h"
#include <string>

using namespace cv;

class CvuiCheckbox : public CvuiControl{

public:
	CvuiCheckbox(Mat* window, int x, int y, std::string label, std::function<void(CvuiObject*, double)> onClick);
	void render();
	int getType();
	bool isChecked();
	void setChecked(bool checked);
	void setColor(unsigned int color);

	std::string label;

private:
	bool checked;
	unsigned int color;

};

