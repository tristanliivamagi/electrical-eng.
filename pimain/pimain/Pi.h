#pragma once


//#include <iostream>
//#include <vector>
//#include <algorithm>
#include <string> 
//#include <sstream>
//#include <iomanip>
//#include <conio.h>  dne

#include <thread>
//#include <mutex>
//#include <chrono>


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


	void vision();
	static void vision_thread(CPi* ptr);
	void command();
	static void command_thread(CPi* ptr);
	void start();
	void run();
};

