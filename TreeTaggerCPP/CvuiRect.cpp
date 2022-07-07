#include "CvuiRect.h"
#include "cvui.h"

using namespace cv;

#define RECT_TYPE 6

CvuiRect::CvuiRect(Mat* window, int x, int y, int w, int h, unsigned int borderColor, unsigned int fillColor) : CvuiObject::CvuiObject(window, x, y){
	this->setWidth(w);
	this->setHeight(h);
	this->setBorderColor(borderColor);
	this->setFillColor(fillColor);
}

void CvuiRect::render(){
	cvui::rect(*(this->window), this->x, this->y, this->width, this->height, this->borderColor, this->fillColor);
}

int CvuiRect::getType(){
	return RECT_TYPE;
}

int CvuiRect::getWidth() {
	return this->width;
}

int CvuiRect::getHeight() {
	return this->height;
}

void CvuiRect::setWidth(int w) {
	if (w + this->x <= this->window->cols) {
		this->width = w;
	}
	else {
		this->width = 0;
	}
}

void CvuiRect::setHeight(int h) {

	this->height = h;

	/*if (h + this->y <= this->window->rows) {
		this->height = h;
	}
	else {
		this->height = 0;
	}*/
}

void CvuiRect::setBorderColor(unsigned int color){
	if (color < 0x1000000) {
		this->borderColor = color;
	}
	else {
		this->borderColor = 0xCECECE;
	}
}

void CvuiRect::setFillColor(unsigned int color){
	if (color < 0x1000000) {
		this->fillColor = color;
	}
	else {
		this->fillColor = 0xCECECE;
	}
}

unsigned int CvuiRect::getBorderColor(){
	return this->borderColor;
}

unsigned int CvuiRect::getFillColor(){
	return this->fillColor;
}
