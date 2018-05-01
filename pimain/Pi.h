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



class CPi
{
public:
	CPi();
	~CPi();
	//std::mutex _mutex;
	bool _thread_exit;

	cv::Mat image;
	cv::Mat frame, edges, HSV, threshold;

	raspicam::RaspiCam_Cv Camera;

	void vision();
	void vision_cal();
	void vision_go();

	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector< cv::Point>> contours;
	std::vector<cv::Point> contour;

	cv::Point objpoint;
	float objrad;

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


	//static void vision_thread(CPi* ptr);
	void command();
	/*static void command_thread(CPi* ptr);
	void start();
	void run();*/
};

