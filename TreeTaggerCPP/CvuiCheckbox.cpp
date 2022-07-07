#include "CvuiCheckbox.h"
#include "cvui.h"

#define CHECKBOX_TYPE 2

CvuiCheckbox::CvuiCheckbox(Mat* window, int x, int y, std::string label, std::function<void(CvuiObject*, double)> onClick) : CvuiControl::CvuiControl(window, x, y, onClick){
    this->color = 0xCECECE;
    this->checked = false;
    this->label = label;
}

void CvuiCheckbox::render(){
    bool temp = cvui::checkbox(*(this->window), this->x, this->y, this->label, &(this->checked), this->color);

    if (this->checked != temp) {
        this->checked = temp;

        if (onAction != nullptr) {
            (onAction)(this, NULL);
        }
    }
}

int CvuiCheckbox::getType(){
    return CHECKBOX_TYPE;
}

bool CvuiCheckbox::isChecked(){
    return this->checked;
}

void CvuiCheckbox::setChecked(bool checked){
    this->checked = checked;
}

void CvuiCheckbox::setColor(unsigned int color){
    if (color < 0x1000000) {
        this->color = color;
    }
    else{
        this->color = 0xCECECE;
    }
}
