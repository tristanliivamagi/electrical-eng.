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

	void vision_cal(cv::Mat frame);
	void vision_go(cv::Mat frame);

	cv::Mat image;
	cv::Mat frame, edges, HSV, threshold;

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


	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector< cv::Point>> contours;
	std::vector<cv::Point> contour;

	cv::Point objpoint;
	float objrad;

};

