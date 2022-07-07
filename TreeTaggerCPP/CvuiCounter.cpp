#include "CvuiCounter.h"
#include "cvui.h"

#define COUNTER_TYPE 4

CvuiCounter::CvuiCounter(Mat* window, int x, int y, double defaultValue, double step, std::function<void(CvuiObject*, double)> onChange) : CvuiControl::CvuiControl(window, x, y, onChange) {
	this->value = defaultValue;
	this->lastValue = defaultValue;
	this->step = step;
	this->format = const_cast <char*>("%.2f");
	
}

void CvuiCounter::render(){
	if (lastValue != cvui::counter(*(this->window), this->x, this->y, &(this->value), this->step, this->format)) {
		this->lastValue = this->value;
		(this->onAction)(this, NULL);
	}
}

int CvuiCounter::getType(){
	return COUNTER_TYPE;
}

void CvuiCounter::setFormat(const char* format){
	if (format[0] == '%' && (format[strlen(format)-1] == 'd' || format[strlen(format) - 1] == 'f')) {
		this->format = format;
	}
	else {
		this->format = const_cast <char*>("%.2f");
	}
}
