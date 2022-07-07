#include "CvuiControl.h"

CvuiControl::CvuiControl(Mat* window, int x, int y, std::function<void(CvuiObject*, double)> onAction) : CvuiObject::CvuiObject(window, x, y) {
	this->onAction = onAction;
}
