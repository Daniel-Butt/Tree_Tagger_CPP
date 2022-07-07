#include "CvuiTrackbar.h"
#include "cvui.h"

#define TRACKBAR_TYPE 4

CvuiTrackbar::CvuiTrackbar(Mat* window, int x, int y, int w, std::function<void(CvuiObject*, double)> onChange) : CvuiControl::CvuiControl(window, x, y, onChange){
	this->setWidth(w);
	this->format = const_cast <char*>("%.1Lf");
	this->value = 5;
	this->min = 0;
	this->max = 10;
	this->segments = 1;
	this->options = 0;
	this->discreteStep = 1;
}

void CvuiTrackbar::render(){
	if (cvui::trackbar(*(this->window), this->x, this->y, this->width, &(this->value), this->min, this->max, this->segments, this->format, this->options, this->discreteStep)) {
		(this->onAction)(this, this->value);
	}
}

int CvuiTrackbar::getType()
{
	return TRACKBAR_TYPE;
}

void CvuiTrackbar::setFormat(const char* format){
	if (format[0] == '%' && format[strlen(format) - 2] == 'L' && format[strlen(format) - 1] == 'f') {
		this->format = format;
		std::cout << this->format;
	}
	else {
		this->format = const_cast <char*>("%.1Lf");
	}
}

int CvuiTrackbar::getWidth() {
	return this->width;
}


void CvuiTrackbar::setWidth(int w) {
	if (w + this->x < this->window->cols) {
		this->width = w;
	}
	else {
		this->width = 0;
	}
}
