#pragma once


//#include <iostream>
#include <vector>
//#include <algorithm>
#include <string> 
//#include <sstream>
#include <iomanip>
//#include <conio.h>  dne

#include <thread>
//#include <mutex>
//#include <chrono>
#include <algorithm>
#include <fstream>

//camera copy

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include <opencv2/opencv.hpp>

#include "raspicam_cv.h"

class CColor
{
public:
	CColor();
	~CColor();
	
	char id;
	std::string filename;
	
	void storehsv();
	void gethsv();
	
	void colorid(char name);
	void vision_cal(cv::Mat myframe);
	int vision_go(cv::Mat myframe);

	cv::Mat myimage;
	cv::Mat myframe, myedges, myHSV, mythreshold;
	cv::Mat thresh1, thresh2; 
	int framewidth;
	int frameheight;
	int H_MIN;
	int H_MAX;
	int S_MIN;
	int S_MAX;
	int V_MIN;
	int V_MAX;
	cv::Rect recthsv, obj_rec;
	std::vector <int> H_box, S_box, V_box;

	int hmax = 179;
	int hsvmin = 0;
	int svmax = 255;
	int offseth;
	int offsets;
	int offsetv;
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector< cv::Point>> contours;
	std::vector<cv::Point> contour;

	cv::Point objpoint;
	float objrad;
	float scaledist = 3750;
	float distance;

};

