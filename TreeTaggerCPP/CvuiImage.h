#pragma once
#include "CvuiObject.h"

using namespace cv;

class CvuiImage : public CvuiObject{

public:
	CvuiImage(Mat* window, int x, int y, Mat* image);
	void render();
	int getType();
	bool mouseOver();
	cv::Point relativeMouseLocation();
	void setImage(Mat* image);
	Mat* getImage();

	bool interactable;
	std::function<void(CvuiImage*)> onClick;
	std::function<void(CvuiImage*)> onDown;
	std::function<void(CvuiImage*)> onOver;
	std::function<void(CvuiImage*)> onOut;

private:
	Mat* image;

};


