#pragma once
#include "CvuiObject.h"

using namespace cv;

class CvuiControl : public CvuiObject{
public:
	CvuiControl(Mat* window, int x, int y, std::function<void(CvuiObject*, double)> onAction);


protected:
	std::function<void(CvuiObject*, double)> onAction;

};

