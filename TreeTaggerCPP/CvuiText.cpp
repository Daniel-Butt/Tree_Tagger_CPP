#include "CvuiText.h"
#include "cvui.h"

#define TEXT_TYPE 3

CvuiText::CvuiText(Mat* window, int x, int y, std::string label) : CvuiObject::CvuiObject(window, x, y) {
	this->label = label;
	this->color = 0xCECECE;
}

CvuiText::CvuiText(Mat* window, int x, int y, std::string label, double fontScale) : CvuiObject::CvuiObject(window, x, y) {
	this->label = label;
	this->color = 0xCECECE;
	this->setFontScale(fontScale);
}

void CvuiText::render(){
	cvui::text(*(this->window), this->x, this->y, this->label, this->fontScale, this->color);
}

int CvuiText::getType(){
	return TEXT_TYPE;
}

void CvuiText::setColor(unsigned int color) {
	if (color < 0x1000000) {
		this->color = color;
	}
	else {
		this->color = 0xCECECE;
	}
}

void CvuiText::setFontScale(double fontScale){
	if (fontScale > 0 && fontScale < 10) {
		this->fontScale = fontScale;
	}
	else{
		fontScale = 0.4;
	}
}

double CvuiText::getFontScale()
{
	return this->fontScale;
}
