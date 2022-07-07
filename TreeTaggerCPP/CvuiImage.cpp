#include "CvuiImage.h"
#include "cvui.h"

using namespace cv;

#define IMAGE_TYPE 0

CvuiImage::CvuiImage(Mat* window, int x, int y, Mat* image) : CvuiObject::CvuiObject(window, x, y){
	this->setImage(image);
	this->interactable = false;
	this->onClick = nullptr;
	this->onDown = nullptr;
	this->onOver = nullptr;
	this->onOut = nullptr;
}

void CvuiImage::render(){
	if (!this->image->empty()) {
		cvui::image(*(this->window), this->x, this->y, *(this->image));

		if (interactable) {
			switch (cvui::iarea(this->x, this->y, this->image->cols, this->image->rows)) {

			case cvui::CLICK: if(this->onClick != nullptr) (this->onClick)(this); break;
			case cvui::DOWN: if (this->onDown != nullptr) (this->onDown)(this); break;
			case cvui::OVER: if (this->onOver != nullptr) (this->onOver)(this); break;
			case cvui::OUT: if (this->onOut != nullptr) (this->onOut)(this); break;

			}
		}
	}
}

int CvuiImage::getType()
{
	return IMAGE_TYPE;
}

bool CvuiImage::mouseOver(){
	Point cursor = cvui::mouse();

	if (cursor.x >= this->x && cursor.x < this->x + this->image->cols &&
		cursor.y >= this->y && cursor.y < this->y + this->image->rows) {
		return true;
	}

	return false;
}

cv::Point CvuiImage::relativeMouseLocation(){
	Point cursor = cvui::mouse();

	return cv::Point(cursor.x - this->x, cursor.y - this->y);
}

void CvuiImage::setImage(Mat* image){
	if (this->x + image->cols < this->window->cols && this->y + image->rows < this->window->cols) {
		this->image = image;
	}
	else {
		this->image = new Mat();
	}
}

Mat* CvuiImage::getImage(){
	return this->image;
}
