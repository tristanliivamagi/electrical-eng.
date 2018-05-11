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

#include "Color.h"
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

	
	CColor red;

	CColor green;
	

	cv::Mat image;
	cv::Mat frame;
	//cv::Mat frame, edges, HSV, threshold;

	raspicam::RaspiCam_Cv Camera;
	void vision();

};

