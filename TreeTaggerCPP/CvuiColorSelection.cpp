#include "CvuiColorSelection.h"
#include "cvui.h"

#define COLORSELECTION_TYPE 8

CvuiColorSelection::CvuiColorSelection(Mat* window, int x, int y, int w, int h, unsigned int color, std::function<void(CvuiObject*, double)> onClick) : CvuiControl::CvuiControl(window, x, y, onClick){
	this->width = w;
	this->height = h;
	this->setColor(color);
	this->tolerance = 0;
	this->borderColor = 0x504e54;

}

void CvuiColorSelection::render(){
	cvui::rect(*(this->window), this->x, this->y, this->width, this->height, this->borderColor, 0x212121);
	cvui::text(*(this->window),this->x + 5, this->y + 3, this->colorStr, (double)(this->height / 35.0), 0x504e54);
	cvui::rect(*(this->window), this->x + this->width - this->height, this->y + 3, this->height - 6, this->height - 6, 0x504e54, this->color);

	if (cvui::iarea(this->x, this->y, this->width, this->height) == cvui::CLICK) {
		(this->onAction)(this, this->tolerance);
	}

}

int CvuiColorSelection::getType(){
	return COLORSELECTION_TYPE;
}

unsigned int CvuiColorSelection::getColor(){
	return this->color;
}

void CvuiColorSelection::setColor(unsigned int color){
	if (color < 0x1000000) {
		this->color = color;
	}
	else {
		this->color = 0xCECECE;
	}

	stringstream stream;

	stream << std::hex << this->color;

	this->colorStr = stream.str();

	for (int i = colorStr.length(); i < 6; i++) {
		colorStr.insert(0, 1, '0');
	}

	colorStr.insert(0, 1, '#');
}

unsigned int CvuiColorSelection::getBorderColor(){
	return this->borderColor;
}

void CvuiColorSelection::setBorderColor(unsigned int color){
	if (color < 0x1000000) {
		this->borderColor = color;
	}
	else {
		this->borderColor = 0xCECECE;
	}
}
