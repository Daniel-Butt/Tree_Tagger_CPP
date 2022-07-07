#pragma once
#include "CvuiObject.h"
#include "CvuiImage.h"
#include "CvuiButton.h"
#include "CvuiCheckbox.h"
#include "CvuiText.h"
#include "CvuiCounter.h"
#include "CvuiTrackbar.h"
#include "CvuiRect.h"
#include "CvuiColorSelection.h"


#include <string>
#include <vector>

using namespace std;

class CvuiVertWindow : public CvuiObject{
public:
	CvuiVertWindow(Mat* window, int x, int y, int w, int h, string title);
	void render();
	int getType();
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);
	void addItem(CvuiObject* item);
	vector<CvuiObject*>* getItems();

	string title;

private:
	int getItemBottom(CvuiObject* item);

	int width;
	int height;
	int lastMouseY;
	int mouseY;
	int paddingY;
	vector<CvuiObject*> items;
	CvuiRect* scrollBarSlider;
	
};

