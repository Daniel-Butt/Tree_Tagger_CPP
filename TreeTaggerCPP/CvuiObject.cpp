#include "CvuiObject.h"

using namespace cv;

CvuiObject::CvuiObject(Mat* window, int x, int y) {
	this->x = x;
	this->y = y;
	this->window = window;

}

void CvuiObject::setX(int x){
	this->x = x;
	/*if (window != nullptr && x >= 0 && x <= (*window).cols) {
		this->x = x;
	}
	else {
		this->x = 0;
	}*/
}

void CvuiObject::setY(int y){
	this->y = y;
	/*if (window != nullptr && y >= 0 && y <= (*window).rows) {
		this->y = y;
	}
	else {
		this->y = 0;
	}*/
}

int CvuiObject::getX(){
	return this->x;
}

int CvuiObject::getY(){
	return this->y;
}
