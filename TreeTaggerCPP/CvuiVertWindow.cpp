#include "CvuiVertWindow.h"
#include "cvui.h"

#define IMAGE_TYPE 0
#define BUTTON_TYPE 1
#define CHECKBOX_TYPE 2
#define TEXT_TYPE 3
#define COUNTER_TYPE 4
#define TRACKBAR_TYPE 5
#define RECT_TYPE 6
#define VERTWINDOW_TYPE 7
#define COLORSELECTION_TYPE 8

CvuiVertWindow::CvuiVertWindow(Mat* window, int x, int y, int w, int h, string title) : CvuiObject::CvuiObject(window, x, y){
	this->setWidth(w);
	this->setHeight(h);
	this->title = title;
	this->scrollBarSlider = new CvuiRect(window, this->x + this->width - 14, this->y + 21, 13, this->height - 22, 0x212121, 0x504e54);
	this->lastMouseY = -1;
	this->mouseY = -1;
	this->paddingY = -1;

}

void CvuiVertWindow::render(){
	cvui::window(*(this->window), this->x, this->y, this->width, this->height, this->title);
	cvui::rect(*(this->window), this->x + this->width - 15, this->y + 20, 15, this->height - 20, 0x504e54, 0x212121);

	if (cvui::iarea(this->scrollBarSlider->getX()-2, this->scrollBarSlider->getY(), this->scrollBarSlider->getWidth()+4, this->scrollBarSlider->getHeight()) == cvui::DOWN) {

		this->scrollBarSlider->setBorderColor(0x313131);
		this->scrollBarSlider->setFillColor(0x605e64);

		if (lastMouseY == -1) {
			lastMouseY = cvui::mouse().y;
		}
		else {
			mouseY = cvui::mouse().y;

			if (mouseY != lastMouseY) {
				int newScrollbarY = this->scrollBarSlider->getY() + (mouseY - lastMouseY);

				if (newScrollbarY < this->y + 21) {
					newScrollbarY = this->y + 21;
				}
				else if (newScrollbarY + this->scrollBarSlider->getHeight() > this->y + this->height - 1) {
					newScrollbarY = this->y + this->height - 1 - this->scrollBarSlider->getHeight();
				}

				for (CvuiObject* item : items) {
					item->setY(item->getY() + (this->scrollBarSlider->getY() - newScrollbarY)*2);
				}

				this->scrollBarSlider->setY(newScrollbarY);

				lastMouseY = mouseY;
			}
		}
	}
	else {
		lastMouseY = -1;
		this->scrollBarSlider->setBorderColor(0x212121);
		this->scrollBarSlider->setFillColor(0x504e54);
	}

	this->scrollBarSlider->render();

	for (CvuiObject* item : items) {
		
		if (item->getY() >= this->y + 21 && this->getItemBottom(item) < this->y + this->height) {
			item->render();
		}
		
	}
}

int CvuiVertWindow::getType(){

	return VERTWINDOW_TYPE;
}

int CvuiVertWindow::getWidth(){
	return this->width;
}

int CvuiVertWindow::getHeight(){
	return this->height;
}

void CvuiVertWindow::setWidth(int w){
	if (w + this->x <= this->window->cols) {
		this->width = w;
	}
	else {
		this->width = 0;
	}
}

void CvuiVertWindow::setHeight(int h){
	if (h + this->y <= this->window->rows) {
		this->height = h;
	}
	else {
		this->height = 0;
	}
}

void CvuiVertWindow::addItem(CvuiObject* item){

	if (this->paddingY == -1) paddingY = item->getY();

	//int oldY = item->getY();

	item->setX(this->x + item->getX());

	if (this->items.size() == 0) {
		item->setY(this->y + 20 + item->getY());
	}
	else {

		CvuiObject* lastItem = items.at(items.size() - 1);
		int lastBottom = this->getItemBottom(lastItem);
		
		item->setY(item->getY() + lastBottom);
	}

	items.push_back(item);

	int newItemBottom = this->getItemBottom(item);
	int firstItemTop = items.at(0)->getY();

	if (newItemBottom > this->y + this->height) {

		//int newHeight = (this->height - 22) - oldY - (int)ceil((((double)(newItemBottom + firstItemTopOffset) - this->y - this->height) / 2.0));
		int newHeight = (this->height - 22) - paddingY - (int)ceil(((double)(newItemBottom - firstItemTop - this->height)) / 2.0);

		this->scrollBarSlider->setHeight(newHeight);

	}
}

int CvuiVertWindow::getItemBottom(CvuiObject* item) {

	int bottom = 0;

	switch (item->getType()) {
	case IMAGE_TYPE: {
		CvuiImage* image = static_cast<CvuiImage*>(item);
		bottom += image->getImage()->rows + image->getY();
		break;
	}
	case BUTTON_TYPE: {
		CvuiButton* button = static_cast<CvuiButton*>(item);
		bottom += button->getHeight() + button->getY();
		break;
	}
	case TEXT_TYPE: {
		CvuiText* text = static_cast<CvuiText*>(item);
		bottom += text->getFontScale() * 35 + text->getY();
		break;
	}
	case RECT_TYPE: {
		CvuiRect* rect = static_cast<CvuiRect*>(item);
		bottom += rect->getHeight() + rect->getY();
		break;
	}

	case COLORSELECTION_TYPE: {
		CvuiColorSelection* cs = static_cast<CvuiColorSelection*>(item);
		bottom += cs->height + cs->getY();
		break;
	}
	}

	return bottom;
}

vector<CvuiObject*>* CvuiVertWindow::getItems(){
	return &(this->items);
}
