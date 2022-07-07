#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <winuser.h>
#include <shobjidl.h>
#include <chrono>
#include <locale>
#include <filesystem>
#include <time.h>
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/photo/cuda.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ximgproc.hpp>

#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include "CvuiObject.h"
#include "CvuiImage.h"
#include "CvuiButton.h"
#include "CvuiCheckbox.h"
#include "CvuiText.h"
#include "CvuiCounter.h"
#include "CvuiTrackbar.h"
#include "CvuiRect.h"
#include "CvuiVertWindow.h"
#include "CvuiColorSelection.h"

#define WINDOW_NAME "Tree Tagger 0.2"
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1017
#define VERSION 0.2

#define IMAGE_TYPE 0
#define BUTTON_TYPE 1
#define CHECKBOX_TYPE 2
#define TEXT_TYPE 3
#define COUNTER_TYPE 4
#define TRACKBAR_TYPE 5
#define RECT_TYPE 6
#define VERTWINDOW_TYPE 7
#define COLORSELECTION_TYPE 8

#define NORMAL_MODE 0
#define BRIGHTNESS_MODE 1
#define CONTRAST_MODE 2
#define COLOR_MODE 3
#define NEWCOLORSELECT_MODE 4
#define COLORSELECT_MODE 5
#define DENOISE_MODE 6
#define BLUR_MODE 7
#define THINNING_MODE 8
#define FINDLINES_MODE 9
#define HOUGHP_MODE 10
#define ERASE_MODE 11

#define PI 3.14159265358979323846264338327950

//#define IDI_ICON1 101
//#define IDC_CURSOR1 102

#include "resource.h"

using namespace std;
using namespace cv;

cv::Mat mainWindow;

cv::Mat* mainImg;
cv::Mat* filteredImg;
cv::Mat* filteredImgBackup;
cv::Mat* newFilteredImg;
cv::Mat* rMainImg;
cv::Mat* rFilteredImg;
cuda::GpuMat* gpuFilteredImg;

vector<Mat*> filteredImgs;
vector<Mat*> vwrFilteredImgs;

vector<CvuiObject*> staticUiObjects;
vector<CvuiObject*> dynamicUiObjects;

CvuiVertWindow* appliedVertWindow;

CvuiColorSelection* selectedColorSelect;

vector<Mat*> colorMasks;

vector<Vec4i> lines;
vector<Vec4f> linesf;

int eraserSize;

//unsigned int selectedColor;

int mode;

struct {
	cv::Point pos;
	int width;
	int height;
	double zoomScale;
} mainDisplayFrame, filteredDisplayFrame;

void openFile(CvuiObject* caller, double value){
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						//MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
						wstring ws(pszFilePath);
						std::string filePath = string(ws.begin(), ws.end());
						
						*mainImg = imread(filePath, IMREAD_UNCHANGED);
						if (mainImg->type() == CV_8UC3) {
							cv::cvtColor(*mainImg, *mainImg, COLOR_BGR2BGRA);
						}
						else if (mainImg->type() == CV_8UC1) {
							cv::cvtColor(*mainImg, *mainImg, COLOR_GRAY2BGRA);
						}

						*filteredImg = mainImg->clone();
						filteredImgBackup = filteredImg;
						resize(*mainImg, *rMainImg, Size(750, 750));
						*rFilteredImg = rMainImg->clone();
						filteredImgs.push_back(new Mat(mainImg->clone()));

						gpuFilteredImg->upload(*mainImg);

						//filteredDisplayFrame.pos = cv::Point(filteredImg->cols/2, filteredImg->rows/2);
						filteredDisplayFrame.width = filteredImg->cols;
						filteredDisplayFrame.height = filteredImg->rows;

						//mainDisplayFrame.pos = cv::Point(mainImg->cols / 2, mainImg->rows / 2);
						mainDisplayFrame.width = mainImg->cols;
						mainDisplayFrame.height = mainImg->rows;

						colorMasks.clear();

						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
}

void exportFiles(CvuiObject* caller, double value) {
	vector<Mat*> mainImageSections;
	vector<Mat*> maskImageSections;

	srand((unsigned)time(NULL));

	for (int x = 0; x < 3; x++) {

		mainImageSections.clear();
		maskImageSections.clear();

		int size = 256 + x * 128;

		std::cout << size << endl;
		std::cout << filteredImgBackup->cols / size << ", " << filteredImgBackup->rows / size << endl;

		Mat gray = Mat();
		cv::cvtColor(*filteredImgBackup, gray, COLOR_BGRA2GRAY);

		//rescale for segmentation, 0 = background class, 1 = tree class
		gray = gray / 255;

		for (int i = 0; i < filteredImgBackup->rows; i += size) {
			for (int j = 0; j < filteredImgBackup->cols; j += size) {
				mainImageSections.push_back(new Mat(*mainImg, cv::Rect(j, i, size, size)));

				maskImageSections.push_back(new Mat(gray, cv::Rect(j, i, size, size)));

				//Data Augment
				int randX = rand() % (filteredImgBackup->cols - size);
				int randY = rand() % (filteredImgBackup->rows - size);

				mainImageSections.push_back(new Mat(*mainImg, cv::Rect(randX, randY, size, size)));

				maskImageSections.push_back(new Mat(gray, cv::Rect(randX, randY, size, size)));
			}
		}

		string MLImgsFolder = "images/ML_Images_" + std::to_string(size);
		std::cout << MLImgsFolder << endl;
		string MLMasksFolder = "images/ML_Masks_" + std::to_string(size);
		std::cout << MLMasksFolder << endl;


		if (std::filesystem::is_empty(MLImgsFolder)) {
			for (int i = 0; i < mainImageSections.size(); i++) {
				cv::imwrite(MLImgsFolder + "/img" + std::to_string(i) + ".bmp", *(mainImageSections.at(i)));
				cv::imwrite(MLMasksFolder + "/img" + std::to_string(i) + ".bmp", *(maskImageSections.at(i)));
			}
		}
		else {

			int offset = 0;

			for (auto file : std::filesystem::directory_iterator(MLImgsFolder)) {

				string path = file.path().string();

				int idx = std::stoi(path.substr(MLImgsFolder.length() + 4, path.length() - 4 - MLImgsFolder.length()));

				offset = idx > offset ? idx : offset;
			}

			offset++;

			for (int i = 0; i < mainImageSections.size(); i++) {
				cv::imwrite(MLImgsFolder + "/img" + std::to_string(i + offset) + ".bmp", *(mainImageSections.at(i)));
				cv::imwrite(MLMasksFolder + "/img" + std::to_string(i + offset) + ".bmp", *(maskImageSections.at(i)));
			}

		}
	}

}

void changeMode(CvuiObject* caller, double value) {}

void openSettings(CvuiObject* caller, double value) {}

void openHelp(CvuiObject* caller, double value) {}

void updateDisplay() {
	*rFilteredImg = Mat(*filteredImg, cv::Rect(filteredDisplayFrame.pos.x, filteredDisplayFrame.pos.y, filteredDisplayFrame.width, filteredDisplayFrame.height));

	if (filteredDisplayFrame.width <= 750) {
		cv::resize(*rFilteredImg, *rFilteredImg, cv::Size(750, 750), INTER_CUBIC);
	}
	else {
		cv::resize(*rFilteredImg, *rFilteredImg, cv::Size(750, 750), INTER_AREA);
	}
}

void removeLast(CvuiObject* caller, double value) {
	if (filteredImgs.size() < 2) return;

	filteredImgs.pop_back();
	vwrFilteredImgs.pop_back();
	appliedVertWindow->getItems()->pop_back();
	appliedVertWindow->getItems()->pop_back();

	filteredImg = filteredImgBackup;

	*filteredImg = *filteredImgs.back();
	gpuFilteredImg->upload(*filteredImg);

	updateDisplay();

}

void overlayLines(CvuiObject* caller, double value) {
	*newFilteredImg = mainImg->clone();

	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(*newFilteredImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, LINE_AA);
	}

	filteredImg = newFilteredImg;

	updateDisplay();
}

void vwrImageClicked(CvuiImage* caller) {

	std::vector<Mat*>::iterator itr = std::find(vwrFilteredImgs.begin(), vwrFilteredImgs.end(), caller->getImage());

	int idx = std::distance(vwrFilteredImgs.begin(), itr) + 1;

	filteredImg = filteredImgs.at(idx);

	//cv::resize(*filteredImg, *rFilteredImg, cv::Size(750, 750));
	updateDisplay();

}

void applyFilter(CvuiObject* caller, double value) {

	//filteredImg = filteredImgBackup;

	if (!filteredImg->empty()) {
		vwrFilteredImgs.push_back(new Mat());

		cv::resize(*filteredImg, *vwrFilteredImgs.at(vwrFilteredImgs.size() - 1), cv::Size(160, 160), 0, 0, cv::INTER_AREA);

		appliedVertWindow->addItem(new CvuiText(&mainWindow, 15, 30, "TODO", 0.5));

		CvuiImage* vwrImage = new CvuiImage(&mainWindow, 15, 5, vwrFilteredImgs.at(vwrFilteredImgs.size() - 1));
		vwrImage->interactable = true;
		vwrImage->onClick = std::bind(&vwrImageClicked, std::placeholders::_1);

		appliedVertWindow->addItem(vwrImage);

		filteredImgs.push_back(new Mat(filteredImg->clone()));
		gpuFilteredImg->upload(*filteredImg);

		if (filteredImg == newFilteredImg) {
			*filteredImgBackup = filteredImg->clone();
			filteredImg = filteredImgBackup;
		}
	}
	
}

void setupSelectColor(CvuiObject* caller, double value) {
	//from new color button
	if (caller->getType() == BUTTON_TYPE) {
		mode = NEWCOLORSELECT_MODE;
	}
	//from already created color
	else {
		mode = COLORSELECT_MODE;

		if (selectedColorSelect != nullptr)	selectedColorSelect->setBorderColor(0x504e54);

		CvuiColorSelection* cs = static_cast<CvuiColorSelection*>(caller);
		cs->setBorderColor(0xFF0000);

		static_cast<CvuiTrackbar*>(dynamicUiObjects.at(3))->value = cs->tolerance;

		selectedColorSelect = cs;

	}
}

cv::Point lastMouse = cv::Point(-1, -1);

void panfilteredDisplayFrame(CvuiImage* caller) {
	if (mode == NEWCOLORSELECT_MODE) {

		unsigned int color = static_cast<CvuiRect*>(dynamicUiObjects.at(2))->getFillColor();

		static_cast<CvuiVertWindow*>(dynamicUiObjects.at(0))->addItem(new CvuiColorSelection(&mainWindow, 15, 10, 160, 25, color, std::bind(&setupSelectColor, std::placeholders::_1, std::placeholders::_2)));

		mode = COLOR_MODE;
	}
	else if (mode == COLORSELECT_MODE) {

		/*unsigned int color = static_cast<CvuiRect*>(dynamicUiObjects.at(2))->getFillColor();

		selectedColorSelect->setColor(color);
		selectedColorSelect->setBorderColor(0x504e54);

		mode = COLOR_MODE;
		selectedColorSelect = nullptr;*/
	}
	else if (mode == ERASE_MODE) {
		filteredImg = filteredImgBackup;
		int offset = eraserSize / 2;

		Point cursor = caller->relativeMouseLocation();


		cursor.x = filteredDisplayFrame.pos.x + (int)((double)cursor.x * ((double)filteredDisplayFrame.width / 750.0)) - offset;
		cursor.y = filteredDisplayFrame.pos.y + (int)((double)cursor.y * ((double)filteredDisplayFrame.height / 750.0)) - offset;

		cursor.x = cursor.x < offset ? offset + 1 : cursor.x;
		cursor.y = cursor.y < offset ? offset + 1 : cursor.y;

		cursor.x = cursor.x > filteredImg->cols - eraserSize - 1 ? filteredImg->cols - eraserSize - 1 : cursor.x;

		cursor.y = cursor.y > filteredImg->rows - eraserSize - 1 ? filteredImg->rows - eraserSize - 1 : cursor.y;

		if (cvui::mouse(cvui::LEFT_BUTTON, cvui::IS_DOWN)) {

			Mat* roi = new Mat(*filteredImg, Rect(cursor.x, cursor.y, eraserSize, eraserSize));
			if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(1))->isChecked()) {
				*roi = cv::Scalar(255, 255, 255);
			}
			else { 
				*roi = cv::Scalar(0);
			}

			updateDisplay();
			return;
		}

	}

	if (lastMouse.x == -1) {
		lastMouse = cvui::mouse(WINDOW_NAME);
	}
	else {
		cv::Point currentMouse = cvui::mouse(WINDOW_NAME);
		if (lastMouse.x != currentMouse.x || lastMouse.y != currentMouse.y) {
			int deltaX = lastMouse.x - currentMouse.x;
			int deltaY = lastMouse.y - currentMouse.y;

			filteredDisplayFrame.pos.x += deltaX;
			filteredDisplayFrame.pos.y += deltaY;

			mainDisplayFrame.pos.x += deltaX;
			mainDisplayFrame.pos.y += deltaY;

			if (filteredDisplayFrame.pos.x < 0) filteredDisplayFrame.pos.x = 0;
			else if (filteredDisplayFrame.pos.x + filteredDisplayFrame.width > filteredImg->cols) filteredDisplayFrame.pos.x = filteredImg->cols - filteredDisplayFrame.width;
			if (filteredDisplayFrame.pos.y < 0) filteredDisplayFrame.pos.y = 0;
			else if (filteredDisplayFrame.pos.y + filteredDisplayFrame.height > filteredImg->rows) filteredDisplayFrame.pos.y = filteredImg->rows - filteredDisplayFrame.height;

			if (mainDisplayFrame.pos.x < 0) mainDisplayFrame.pos.x = 0;
			else if (mainDisplayFrame.pos.x + mainDisplayFrame.width > mainImg->cols) mainDisplayFrame.pos.x = mainImg->cols - mainDisplayFrame.width;
			if (mainDisplayFrame.pos.y < 0) mainDisplayFrame.pos.y = 0;
			else if (mainDisplayFrame.pos.y + mainDisplayFrame.height > mainImg->rows) mainDisplayFrame.pos.y = mainImg->rows - mainDisplayFrame.height;

			*rFilteredImg = Mat(*filteredImg, cv::Rect(filteredDisplayFrame.pos.x, filteredDisplayFrame.pos.y, filteredDisplayFrame.width, filteredDisplayFrame.height));

			cv::resize(*rFilteredImg, *rFilteredImg, cv::Size(750, 750));

			*rMainImg = Mat(*mainImg, cv::Rect(mainDisplayFrame.pos.x, mainDisplayFrame.pos.y, mainDisplayFrame.width, mainDisplayFrame.height));

			cv::resize(*rMainImg, *rMainImg, cv::Size(750, 750));

			lastMouse = currentMouse;
		}
	}
}

void zoomfilteredDisplayFrame(CvuiImage* caller) {

	if (mode == COLORSELECT_MODE || mode == NEWCOLORSELECT_MODE) {
		if (filteredImg->type() == CV_8UC4) {
			cv::Vec4b color = rFilteredImg->at<Vec4b>(caller->relativeMouseLocation());

			static_cast<CvuiRect*>(dynamicUiObjects.at(2))->setFillColor(color[0] | (color[1] << 8) | (color[2] << 16));
		}
	}

	lastMouse.x = -1;

	int wheelDelta = cvui::internal::getContext(WINDOW_NAME).mouse.wheel_delta / 120;

	if (wheelDelta != 0) {

		cvui::internal::getContext(WINDOW_NAME).mouse.wheel_delta = 0;

		double oldScale = filteredDisplayFrame.zoomScale;

		filteredDisplayFrame.zoomScale -= 0.1 * wheelDelta;
		mainDisplayFrame.zoomScale -= 0.1 * wheelDelta;

		if (filteredDisplayFrame.zoomScale > 1) filteredDisplayFrame.zoomScale = 1;
		if (filteredDisplayFrame.zoomScale < 0.1) filteredDisplayFrame.zoomScale = 0.1;
		if (mainDisplayFrame.zoomScale > 1) mainDisplayFrame.zoomScale = 1;
		if (mainDisplayFrame.zoomScale < 0.1) mainDisplayFrame.zoomScale = 0.1;

		if (oldScale != filteredDisplayFrame.zoomScale) {

			filteredDisplayFrame.pos.x += filteredDisplayFrame.width/2;
			filteredDisplayFrame.pos.y += filteredDisplayFrame.height/2;

			filteredDisplayFrame.width = filteredImg->cols * filteredDisplayFrame.zoomScale;
			filteredDisplayFrame.height = filteredImg->rows * filteredDisplayFrame.zoomScale;

			filteredDisplayFrame.pos.x -= filteredDisplayFrame.width/2;
			filteredDisplayFrame.pos.y -= filteredDisplayFrame.height/2;

			mainDisplayFrame.pos.x += mainDisplayFrame.width / 2;
			mainDisplayFrame.pos.y += mainDisplayFrame.height / 2;

			mainDisplayFrame.width = mainImg->cols * mainDisplayFrame.zoomScale;
			mainDisplayFrame.height = mainImg->rows * mainDisplayFrame.zoomScale;

			mainDisplayFrame.pos.x -= mainDisplayFrame.width / 2;
			mainDisplayFrame.pos.y -= mainDisplayFrame.height / 2;

			if (filteredDisplayFrame.pos.x < 0) filteredDisplayFrame.pos.x = 0;
			else if (filteredDisplayFrame.pos.x + filteredDisplayFrame.width > filteredImg->cols) filteredDisplayFrame.pos.x = filteredImg->cols - filteredDisplayFrame.width;
			if (filteredDisplayFrame.pos.y < 0) filteredDisplayFrame.pos.y = 0;
			else if (filteredDisplayFrame.pos.y + filteredDisplayFrame.height > filteredImg->rows) filteredDisplayFrame.pos.y = filteredImg->rows - filteredDisplayFrame.height;

			if (mainDisplayFrame.pos.x < 0) mainDisplayFrame.pos.x = 0;
			else if (mainDisplayFrame.pos.x + mainDisplayFrame.width > mainImg->cols) mainDisplayFrame.pos.x = mainImg->cols - mainDisplayFrame.width;
			if (mainDisplayFrame.pos.y < 0) mainDisplayFrame.pos.y = 0;
			else if (mainDisplayFrame.pos.y + mainDisplayFrame.height > mainImg->rows) mainDisplayFrame.pos.y = mainImg->rows - mainDisplayFrame.height;

			*rFilteredImg = Mat(*filteredImg, cv::Rect(filteredDisplayFrame.pos.x, filteredDisplayFrame.pos.y, filteredDisplayFrame.width, filteredDisplayFrame.height));

			cv::resize(*rFilteredImg, *rFilteredImg, cv::Size(750, 750));

			*rMainImg = Mat(*mainImg, cv::Rect(mainDisplayFrame.pos.x, mainDisplayFrame.pos.y, mainDisplayFrame.width, mainDisplayFrame.height));

			cv::resize(*rMainImg, *rMainImg, cv::Size(750, 750));
		}
	}

}

void resetMode() {
	*filteredImg = filteredImgs.at(filteredImgs.size() - 1)->clone();
	gpuFilteredImg->upload(*filteredImg);

	updateDisplay();

	dynamicUiObjects.clear();
}

void changeBrightness(CvuiObject* caller, double value) {

	//reset filteredImg pointer to last filteredImg
	filteredImg = filteredImgBackup;

	//image processing
	filteredImg->convertTo(*newFilteredImg, -1, 1, value);

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();
}

void setupBrightness(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = BRIGHTNESS_MODE;

	resetMode();

	CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 400, 80, 1120, std::bind(&changeBrightness, std::placeholders::_1, std::placeholders::_2));

	tb->value = 0;
	tb->min = -255;
	tb->max = 255;
	tb->segments = 2;

	dynamicUiObjects.push_back(tb);
}

void changeContrast(CvuiObject* caller, double value) {
	//reset filteredImg pointer to last filteredImg
	filteredImg = filteredImgBackup;

	//image processing
	filteredImg->convertTo(*newFilteredImg, -1, value, 0);

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();
}

void setupContrast(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = CONTRAST_MODE;

	resetMode();

	CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 400, 80, 1120, std::bind(&changeContrast, std::placeholders::_1, std::placeholders::_2));

	tb->value = 1;
	tb->min = 0;
	tb->max = 8;
	tb->segments = 2;

	dynamicUiObjects.push_back(tb);
}

void changeColor(CvuiObject* caller, double value) {
	if (selectedColorSelect == nullptr) return;

	vector<CvuiObject*>* vwItems = static_cast<CvuiVertWindow*>(dynamicUiObjects.at(0))->getItems();

	std::vector<CvuiObject*>::iterator itr = std::find(vwItems->begin(), vwItems->end(), selectedColorSelect);

	int idx = std::distance(vwItems->begin(), itr);

	unsigned int color = selectedColorSelect->getColor();
	int tolerance = value;
	selectedColorSelect->tolerance = tolerance;

	cv::Vec3i colorVec = Vec3i(color & 0x0000FF, (color & 0x00FF00)>>8, (color & 0xFF0000)>>16);

	//cout << filteredImg->row(0).col(0) << endl;

	cv::Scalar lower = Scalar(max(0, colorVec[0] - tolerance), max(0, colorVec[1] - tolerance), max(0, colorVec[2] - tolerance), 0);
	cv::Scalar upper = Scalar(min(255, colorVec[0] + tolerance), min(255, colorVec[1] + tolerance), min(255, colorVec[2] + tolerance), 255);

	//cout << lower << endl;
	//cout << upper << endl;

	cuda::GpuMat gMat = cuda::GpuMat();

	cuda::inRange(*gpuFilteredImg, lower, upper, gMat);

	Mat* mask = new Mat();

	gMat.download(*mask);

	if (idx >= colorMasks.size()) {
		colorMasks.push_back(mask);
	}
	else {
		*(colorMasks.at(idx)) = *mask;
	}

	*newFilteredImg = Mat(mask->rows, mask->cols, mask->type(), cv::Scalar(0));

	for (Mat* m : colorMasks) {
		cv::bitwise_or(*newFilteredImg, *m, *newFilteredImg);
	}

	cv::cvtColor(*newFilteredImg, *newFilteredImg, COLOR_GRAY2BGRA);

	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();

}

void setupColor(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = COLOR_MODE;

	resetMode();

	CvuiVertWindow* vw = new CvuiVertWindow(&mainWindow, 250, 50, 200, 150, "Color Selection");

	dynamicUiObjects.push_back(vw);

	dynamicUiObjects.push_back(new CvuiButton(&mainWindow, 460, 65, 100, 25, "new color", std::bind(&setupSelectColor, std::placeholders::_1, std::placeholders::_2)));

	dynamicUiObjects.push_back(new CvuiRect(&mainWindow, 480, 95, 60, 60, 0x504e54, 0x313131));

	CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 600, 80, 600, std::bind(&changeColor, std::placeholders::_1, std::placeholders::_2));
	tb->value = 0;
	tb->min = 0;
	tb->max = 100;

	dynamicUiObjects.push_back(tb);
}

//void selectColor(CvuiObject* caller, double value) {}

void changeDenoise(CvuiObject* caller, double value) {

	//image processing

	cuda::GpuMat gMat = cuda::GpuMat();

	cuda::cvtColor(*gpuFilteredImg, gMat, COLOR_BGRA2GRAY);

	if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(1))->isChecked()) {
		cuda::fastNlMeansDenoising(gMat, gMat, value, 15, 5);
	}
	else {
		cuda::fastNlMeansDenoising(gMat, gMat, value);
	}

	cuda::cvtColor(gMat, gMat, COLOR_GRAY2BGRA);

	gMat.download(*newFilteredImg);

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();
}

void setupDenoise(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = DENOISE_MODE;

	resetMode();

	CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 400, 80, 1120, std::bind(&changeDenoise, std::placeholders::_1, std::placeholders::_2));

	tb->value = 0;
	tb->min = 0;
	tb->max = 100;
	tb->segments = 2;

	dynamicUiObjects.push_back(tb);

	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 100, "large image?", std::bind(&changeDenoise, std::placeholders::_1, std::placeholders::_2)));
}

void changeBlur(CvuiObject* caller, double value) {
	if (!mainImg->empty()) {
		int k = static_cast<CvuiTrackbar*>(dynamicUiObjects.at(0))->value;

		//filteredImg = filteredImgBackup;

		cuda::GpuMat gMat = cuda::GpuMat();

		Ptr<cuda::Filter> blurFilter;
		
		if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(2))->isChecked()) {
			blurFilter = cuda::createBoxMaxFilter(CV_8UC4, cv::Size(k, k));
		}
		else if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(3))->isChecked()) {
			blurFilter = cuda::createBoxMinFilter(CV_8UC4, cv::Size(k, k));
		}
		else if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(4))->isChecked()) {
			cuda::cvtColor(*gpuFilteredImg, gMat, COLOR_BGRA2GRAY);

			cuda::threshold(gMat, gMat, 255 / 2.0, 255, THRESH_BINARY);

			gMat.download(*newFilteredImg);

			cv::cvtColor(*newFilteredImg, *newFilteredImg, COLOR_GRAY2BGRA);

			filteredImg = newFilteredImg;

			updateDisplay();

			return;
		}
		else {
			blurFilter = cuda::createBoxFilter(CV_8UC4, CV_8UC4, cv::Size(k, k));
		}

		blurFilter->apply(*gpuFilteredImg, gMat);

		gMat.download(*newFilteredImg);
		
		filteredImg = newFilteredImg;

		updateDisplay();
	}
}

void setupBlur(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = BLUR_MODE;

	resetMode();

	CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 400, 80, 1120, std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2));

	tb->value = 2;
	tb->min = 2;
	tb->max = 8;
	tb->segments = 2;

	dynamicUiObjects.push_back(tb);

	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 50, "mean", std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2)));
	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 100, "max", std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2)));
	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 150, "min", std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2)));
	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 200, "binary", std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2)));
}

void thinning(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = THINNING_MODE;

	resetMode();

	//reset filteredImg pointer to last filteredImg
	filteredImg = filteredImgBackup;

	//image processing

	auto start = std::chrono::system_clock::now();

	cv::cvtColor(*filteredImg, *newFilteredImg, COLOR_BGRA2GRAY);

	cv::ximgproc::thinning(*newFilteredImg, *newFilteredImg, cv::ximgproc::THINNING_GUOHALL);

	cv::cvtColor(*newFilteredImg, *newFilteredImg, COLOR_GRAY2BGRA);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << elapsed.count() << '\n';

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();

}

void findLines(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = FINDLINES_MODE;

	resetMode();

	//reset filteredImg pointer to last filteredImg
	filteredImg = filteredImgBackup;

	//image processing
	cv::cvtColor(*filteredImg, *newFilteredImg, COLOR_BGRA2GRAY);

	Ptr<cv::ximgproc::FastLineDetector> lsd = cv::ximgproc::createFastLineDetector(10, 1.41421356, 50, 50, 3, true);

	lsd->detect(*newFilteredImg, linesf);

	*newFilteredImg = Mat(filteredImg->rows, filteredImg->cols, CV_8UC3, cv::Scalar(0));

	lsd->drawSegments(*newFilteredImg, linesf);

	lines.clear();

	for (Vec4f l : linesf) {
		lines.push_back(Vec4i(l[0], l[1], l[2], l[3]));
	}

	cv::cvtColor(*newFilteredImg, *newFilteredImg, COLOR_BGR2BGRA);

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();
}

void changeHoughP(CvuiObject* caller, double value) {

	double minLineLength = static_cast<CvuiTrackbar*>(dynamicUiObjects.at(1))->value;
	double maxLineGap = static_cast<CvuiTrackbar*>(dynamicUiObjects.at(2))->value;

	//gpu version doesnt consider a threshold for some reason
	if (static_cast<CvuiCheckbox*>(dynamicUiObjects.at(3))->isChecked()) {
		cuda::GpuMat gLines;
		cuda::GpuMat gMat;

		cuda::cvtColor(*gpuFilteredImg, gMat, COLOR_BGRA2GRAY);

		Ptr<cuda::HoughSegmentDetector> hsd = cuda::createHoughSegmentDetector(1, PI / 180.0, minLineLength, maxLineGap);

		hsd->detect(gMat, gLines);

		gLines.download(lines);
	}
	else {
		int threshold = static_cast<CvuiTrackbar*>(dynamicUiObjects.at(0))->value;

		filteredImg = filteredImgBackup;

		cv::cvtColor(*filteredImg, *newFilteredImg, COLOR_BGRA2GRAY);

		cv::HoughLinesP(*newFilteredImg, lines, 1, PI / 180.0, threshold, minLineLength, maxLineGap);

	}

	*newFilteredImg = Mat(filteredImg->rows, filteredImg->cols, CV_8UC3, cv::Scalar(0));

	for (int i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(*newFilteredImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, LINE_AA);
	}

	cv::cvtColor(*newFilteredImg, *newFilteredImg, COLOR_BGR2BGRA);

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();
}

void setupHoughP(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = HOUGHP_MODE;

	resetMode();

	CvuiTrackbar* tb1 = new CvuiTrackbar(&mainWindow, 300, 80, 300, std::bind(&changeHoughP, std::placeholders::_1, std::placeholders::_2));

	tb1->value = 50;
	tb1->min = 10;
	tb1->max = 150;
	tb1->segments = 2;

	dynamicUiObjects.push_back(tb1);

	CvuiTrackbar* tb2 = new CvuiTrackbar(&mainWindow, 700, 80, 300, std::bind(&changeHoughP, std::placeholders::_1, std::placeholders::_2));

	tb2->value = 30;
	tb2->min = 10;
	tb2->max = 80;
	tb2->segments = 2;

	dynamicUiObjects.push_back(tb2);

	CvuiTrackbar* tb3 = new CvuiTrackbar(&mainWindow, 1100, 80, 300, std::bind(&changeHoughP, std::placeholders::_1, std::placeholders::_2));

	tb3->value = 15;
	tb3->min = 0;
	tb3->max = 30;
	tb3->segments = 2;

	dynamicUiObjects.push_back(tb3);

	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 150, "Gpu?", std::bind(&changeHoughP, std::placeholders::_1, std::placeholders::_2)));

	dynamicUiObjects.push_back(new CvuiText(&mainWindow, 300, 50, "Detection Threshold", 0.6));
	dynamicUiObjects.push_back(new CvuiText(&mainWindow, 700, 50, "Min Line Length", 0.6));
	dynamicUiObjects.push_back(new CvuiText(&mainWindow, 1100, 50, "Max Line Gap", 0.6));
}

void changeEraseSize(CvuiObject* caller, double value) {

	eraserSize = (int)value;
}

void setupErase(CvuiObject* caller, double value) {
	if (mainImg->empty()) {
		return;
	}

	mode = ERASE_MODE;

	resetMode();

	CvuiTrackbar* tb1 = new CvuiTrackbar(&mainWindow, 300, 80, 600, std::bind(&changeEraseSize, std::placeholders::_1, std::placeholders::_2));

	tb1->value = 15;
	tb1->min = 1;
	tb1->max = 61;
	tb1->segments = 2;
	tb1->discreteStep = 2;
	tb1->options = cvui::TRACKBAR_DISCRETE;
	
	dynamicUiObjects.push_back(tb1);

	dynamicUiObjects.push_back(new CvuiCheckbox(&mainWindow, 210, 100, "draw?", std::bind(&changeEraseSize, std::placeholders::_1, std::placeholders::_2)));
}

double LineSegmentToPointDistance(Point lp1, Point lp2, Point p) {
	const int A = p.x - lp1.x;
	const int B = p.y - lp1.y;
	const int C = lp2.x - lp1.x;
	const int D = lp2.y - lp1.y;

	const int dot = A * C + B * D;
	const int len_sq = C * C + D * D;
	const double param = len_sq != 0 ? dot / len_sq : -1;

	double xx, yy;

	if (param < 0) {
		xx = lp1.x;
		yy = lp1.y;
	}
	else if (param > 1) {
		xx = lp2.x;
		yy = lp2.y;
	}
	else {
		xx = lp1.x + param * C;
		yy = lp1.y + param * D;
	}

	return hypot(p.x - xx, p.y - yy);

	//        double numerator = Math.abs((lp2.x - lp1.x)*(lp1.y - p.y) - (lp1.x - p.x)*(lp2.y - lp1.y));
	//
	//        double denominator = Math.hypot(lp2.x - lp1.x, lp2.y - lp1.y);
	//
	//        return numerator / denominator;
}

double LineSegmentToPointDistance(Vec4i l, Point p) {
	return LineSegmentToPointDistance(Point(l[0], l[1]), Point(l[2], l[3]), p);
}

double sqr(double x) { return x * x; }

double dist2(Point v, Point w) { return sqr((double)v.x - (double)w.x) + sqr((double)v.y - (double)w.y); }

double distToSegment(Point v, Point w, Point p) {
	double l2 = dist2(v, w);
	if (l2 == 0) return std::sqrt(dist2(p, v));
	double t = (((double)p.x - (double)v.x) * ((double)w.x - (double)v.x) + ((double)p.y - (double)v.y) * ((double)w.y - (double)v.y)) / l2;
	t = std::max(0.0, std::min(1.0, t));
	return std::sqrt(dist2(p, Point((double)v.x + t * ((double)w.x - (double)v.x), (double)v.y + t * ((double)w.y - (double)v.y))));
}

double distToSegment(Vec4i l, Point p) {
	return distToSegment(Point(l[0], l[1]), Point(l[2], l[3]), p);
}

void joinLines(CvuiObject* caller, double value) {

	if (mainImg->empty()) {
		return;
	}

	mode = FINDLINES_MODE;

	resetMode();

	//reset filteredImg pointer to last filteredImg
	filteredImg = filteredImgBackup;

	const double slopeThreshold = 0.2;
	const double distThreshold = 8;
	const double minLineLength = 25;

	vector<Vec4i> alLines = lines;
	vector<Vec4i> newLines;
	vector<Vec4i> removedLines;

	for (int i = 0; i < alLines.size(); i++) {
		Vec4i l = alLines.at(i);
		if (l[0] > l[2]) {
			alLines.at(i) = Vec4i(l[2], l[3], l[0], l[1]);
		}
	}

	int i = 0;
	int j = 0;

	while(i < alLines.size()) {
		//cout << "i: " << i << endl;
		while(j < alLines.size()) {
			//cout << "j: " << j << endl;
			if (i == j) { j++; continue; }

			Vec4i li = alLines.at(i);
			Vec4i lj = alLines.at(j);

			/*double slopei = ((double)li[1] - (double)li[3]) / ((double)li[0] - (double)li[2]);
			double slopej = ((double)lj[1] - (double)lj[3]) / ((double)lj[0] - (double)lj[2]);

			if (abs(slopei - slopej) > slopeThreshold) { j++; continue; }*/

			double slopeAnglei = atan(((double)li[3] - (double)li[1]) / ((double)li[2] - (double)li[1] + 0.00001));
			double slopeAnglej = atan(((double)lj[3] - (double)lj[1]) / ((double)lj[2] - (double)lj[1] + 0.00001));

			if (abs(slopeAnglei - slopeAnglej) > slopeThreshold) { j++; continue; }

			////find two closest end points between the two lines
			//double delta = hypot(li[0] - lj[0], li[1] - lj[1]);

			//double nextDelta = hypot(li[0] - lj[2], li[1] - lj[3]);
			//if (nextDelta < delta) {
			//	delta = nextDelta;
			//	pj = Point(lj[2], lj[3]);
			//}

			//nextDelta = hypot(li[2] - lj[0], li[3] - lj[1]);
			//if (nextDelta < delta) {
			//	delta = nextDelta;
			//	pi = Point(li[2], li[3]);
			//	pj = Point(lj[0], lj[1]);
			//}

			//nextDelta = hypot(li[2] - lj[2], li[3] - lj[3]);
			//if (nextDelta < delta) {
			//	//delta = nextDelta;
			//	pi = Point(li[2], li[3]);
			//	pj = Point(lj[2], lj[3]);
			//}

			if (distToSegment(li, Point(lj[0], lj[1])) <= distThreshold || distToSegment(li, Point(lj[2], lj[3])) <= distThreshold ||
				distToSegment(lj, Point(li[0], li[1])) <= distThreshold || distToSegment(lj, Point(li[2], li[3])) <= distThreshold) {

			/*if (sqrt(dist2(Point(lj[0], lj[1]), Point(li[0], li[1]))) <= distThreshold ||
				sqrt(dist2(Point(lj[0], lj[1]), Point(li[2], li[3]))) <= distThreshold ||
				sqrt(dist2(Point(lj[2], lj[3]), Point(li[0], li[1]))) <= distThreshold || 
				sqrt(dist2(Point(lj[2], lj[3]), Point(li[2], li[3]))) <= distThreshold){*/

				//find two farthest end points between the two lines
				Point pi = Point(li[0], li[1]);
				Point pj = Point(lj[0], lj[1]);

				double delta = hypot(li[0] - lj[0], li[1] - lj[1]);

				double nextDelta = hypot(li[0] - lj[2], li[1] - lj[3]);
				if (nextDelta > delta) {
					delta = nextDelta;
					pj = Point(lj[2], lj[3]);
				}

				nextDelta = hypot(li[2] - lj[0], li[3] - lj[1]);
				if (nextDelta > delta) {
					delta = nextDelta;
					pi = Point(li[2], li[3]);
					pj = Point(lj[0], lj[1]);
				}

				nextDelta = hypot(li[2] - lj[2], li[3] - lj[3]);
				if (nextDelta > delta) {
					//delta = nextDelta;
					pi = Point(li[2], li[3]);
					pj = Point(lj[2], lj[3]);
				}
				if (pi.x < pj.x) {
					newLines.push_back(Vec4i(pi.x, pi.y, pj.x, pj.y));
				}
				else {
					newLines.push_back(Vec4i(pj.x, pj.y, pi.x, pi.y));
				}
				alLines.push_back(newLines.at(newLines.size() - 1));

				removedLines.push_back(alLines.at(i));
				removedLines.push_back(alLines.at(j));

				if (i > j) {
					alLines.erase(alLines.begin() + i);
					alLines.erase(alLines.begin() + j);

				}
				else {
					alLines.erase(alLines.begin() + j);
					alLines.erase(alLines.begin() + i);
				}
				i = 0;
				j = 0;
			}
			
			j = j + 1;
		}
		j = 0;
		i = i + 1;
	}

	for (int i = 0; i < alLines.size(); i++) {
		Vec4i l = alLines.at(i);
		if (hypot(l[0] - l[2], l[1] - l[3]) < minLineLength) {
			alLines.erase(alLines.begin() + i);
			i--;
		}
	}

	lines.clear();
	lines = alLines;

	*newFilteredImg = Mat(filteredImg->rows, filteredImg->cols, CV_8UC4, cv::Scalar(0));

	for (Vec4i l : lines) {
		cv::line(*newFilteredImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, LINE_AA);
	}

	//change filteredImg pointer to the image processing result
	filteredImg = newFilteredImg;

	//update image displays
	updateDisplay();

}

void initStaticUI() {
	staticUiObjects.push_back(new CvuiRect(&mainWindow, 0, 0, WINDOW_WIDTH, 35, 0x403e44, 0x403e44));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 0, 0, 80, 35, "Open File", std::bind(&openFile, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 80, 0, 80, 35, "Export", std::bind(&exportFiles, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 160, 0, 120, 35, "Change Mode", std::bind(&changeMode, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 280, 0, 80, 35, "Settings", std::bind(&openSettings, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 360, 0, 80, 35, "Help", std::bind(&openHelp, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiRect(&mainWindow, 0, 35, WINDOW_WIDTH, 1, 0x504e54, 0x504e54));
	staticUiObjects.push_back(new CvuiRect(&mainWindow, 204, 214, 752, 752, 0x403e44, 0x05010f));
	staticUiObjects.push_back(new CvuiRect(&mainWindow, WINDOW_WIDTH - 956, 214, 752, 752, 0x403e44, 0x05010f));

	CvuiImage* img = new CvuiImage(&mainWindow, 205, 215, rMainImg);
	img->interactable = true;
	img->onDown = std::bind(&panfilteredDisplayFrame, std::placeholders::_1);
	img->onOver = std::bind(&zoomfilteredDisplayFrame, std::placeholders::_1);

	staticUiObjects.push_back(img);

	img = new CvuiImage(&mainWindow, WINDOW_WIDTH - 955, 215, rFilteredImg);
	img->interactable = true;
	img->onDown = std::bind(&panfilteredDisplayFrame, std::placeholders::_1);
	img->onOver = std::bind(&zoomfilteredDisplayFrame, std::placeholders::_1);

	staticUiObjects.push_back(img);

	staticUiObjects.push_back(new CvuiButton(&mainWindow, 1340, 975, 150, 35, "Overlay Lines", std::bind(&overlayLines, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 1540, 975, 150, 35, "Apply Filter", std::bind(&applyFilter, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiButton(&mainWindow, 1740, 975, 150, 35, "Remove Last", std::bind(&removeLast, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(new CvuiText(&mainWindow, 500, 165, "Original", 1.5));
	staticUiObjects.push_back(new CvuiText(&mainWindow, 1250, 165, "Filtered", 1.5));

	//CvuiTrackbar* tb = new CvuiTrackbar(&mainWindow, 500, 80, 400, std::bind(&changeBlur, std::placeholders::_1, std::placeholders::_2));
	//tb->options = cvui::TRACKBAR_DISCRETE;

	//staticUiObjects.push_back(tb);

	CvuiVertWindow* vw = new CvuiVertWindow(&mainWindow, 0, 37, 200, WINDOW_HEIGHT - 37, "Filters");
	vw->addItem(new CvuiButton(&mainWindow, 20, 30, 150, 60, "Brightness", std::bind(&setupBrightness, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Contrast", std::bind(&setupContrast, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Color", std::bind(&setupColor, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Denoise", std::bind(&setupDenoise, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Blur", std::bind(&setupBlur, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Erase", std::bind(&setupErase, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Thinning", std::bind(&thinning, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Find Lines", std::bind(&findLines, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "HoughP", std::bind(&setupHoughP, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "Join Lines", std::bind(&joinLines, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "button11", std::bind(&openFile, std::placeholders::_1, std::placeholders::_2)));
	vw->addItem(new CvuiButton(&mainWindow, 20, 40, 150, 60, "button12", std::bind(&openFile, std::placeholders::_1, std::placeholders::_2)));
	staticUiObjects.push_back(vw);

	appliedVertWindow = new CvuiVertWindow(&mainWindow, WINDOW_WIDTH - 200, 37, 200, WINDOW_HEIGHT - 89, "Applied");

	staticUiObjects.push_back(appliedVertWindow);
}


//auto end = std::chrono::system_clock::now();
//auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//std::cout << elapsed.count() << '\n';
//start = std::chrono::system_clock::now();

int main() {
	//initialize cvui
	//watch(WINDOW_NAME, true, false);

	cvui::init(WINDOW_NAME);

	HWND win_handle = FindWindowA(0, WINDOW_NAME);
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(win_handle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(win_handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	ShowWindow(GetConsoleWindow(), SW_SHOW);
	//ShowWindow(GetConsoleWindow(), SW_HIDE);

	mainWindow = cv::Mat(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC4);

	mainImg = new Mat();
	filteredImg = new Mat();
	filteredImgBackup = new Mat();
	newFilteredImg = new Mat();
	rMainImg = new Mat();
	rFilteredImg = new Mat();
	gpuFilteredImg = new cuda::GpuMat();

	eraserSize = 15;

	initStaticUI();

	//selectedColor = 0x313131;

	mode = NORMAL_MODE;

	filteredDisplayFrame.pos = Point(0, 0);
	filteredDisplayFrame.width = 750;
	filteredDisplayFrame.height = 750;
	filteredDisplayFrame.zoomScale = 1;

	mainDisplayFrame.pos = Point(0, 0);
	mainDisplayFrame.width = 750;
	mainDisplayFrame.height = 750;
	mainDisplayFrame.zoomScale = 1;

	//auto start = std::chrono::system_clock::now();

	//forever polling everything since cvui handles events at render time
	try {
		while (cv::getWindowProperty(WINDOW_NAME, 0) >= 0) {
			//draw background
			mainWindow = cv::Scalar(49, 49, 49);

			//render ui objects
			for (CvuiObject* uiObject : staticUiObjects) {
				uiObject->render();
			}

			for (CvuiObject* uiObject : dynamicUiObjects) {
				uiObject->render();
			}

			//render window
			cvui::imshow(WINDOW_NAME, mainWindow);

			//requried to ensure cvui updates correctly
			cv::waitKey(1);
			/*auto end = std::chrono::system_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			std::cout << elapsed.count() << '\n';
			start = std::chrono::system_clock::now();*/
		}
	}
	catch (exception& e) { return 1; }


	return 0;
}



