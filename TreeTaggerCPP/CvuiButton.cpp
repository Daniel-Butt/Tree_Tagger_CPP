#include "CvuiButton.h"
#include "cvui.h"
#include <string>

#define BUTTON_TYPE 1

CvuiButton::CvuiButton(Mat* window, int x, int y, int w, int h, std::string label, std::function<void(CvuiObject*, double)> onClick) : CvuiControl::CvuiControl(window, x, y, onClick){
	this->setWidth(w);
	this->setHeight(h);
	this->label = label;
}

CvuiButton::CvuiButton(Mat* window, int x, int y, std::string label, std::function<void(CvuiObject*, double)> onClick) : CvuiControl::CvuiControl(window, x, y, onClick) {
	this->label = label;
}

void CvuiButton::render(){
	if (cvui::button(*(this->window), this->x, this->y, this->width, this->height, this->label)) {
		(this->onAction)(this, NULL);
	}
}

int CvuiButton::getType(){
	return BUTTON_TYPE;
}

int CvuiButton::getWidth(){
	return this->width;
}

int CvuiButton::getHeight(){
	return this->height;
}

void CvuiButton::setWidth(int w){
	if (w + this->x < this->window->cols) {
		this->width = w;
	}
	else {
		this->width = 0;
	}
}

void CvuiButton::setHeight(int h){
	this->height = h;

	/*if (h + this->y < this->window->rows) {
		this->height = h;
	}
	else {
		this->height = 0;
	}*/
}
