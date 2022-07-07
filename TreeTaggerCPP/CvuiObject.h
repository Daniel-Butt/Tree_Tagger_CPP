#include <opencv2/core.hpp>

#pragma once

using namespace cv;

class CvuiObject{
public:
	CvuiObject(Mat* window, int x, int y);
	virtual void render() = 0;
	virtual int getType() = 0;
	void setX(int x);
	void setY(int y);
	int getX();
	int getY();
	

protected:
	int x;
	int y; 
	Mat* window;
};


