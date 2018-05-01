//#include "stdafx.h"
#include "Pi.h"


CPi::CPi()
{
	H_MIN = 66;
	H_MAX = 69;
	S_MIN = 118;
	S_MAX = 136;
	V_MIN = 114;
	V_MAX = 123;
	
	_thread_exit = false;
	Camera.open();
	
	

}


CPi::~CPi()
{
	
}
void CPi::vision()
{


	while (_thread_exit == false)
	{
		Camera.grab();
		Camera.retrieve(frame);
		cv::cvtColor(frame, HSV, cv::COLOR_BGR2HSV);
		//vision_cal();
		vision_go();
		cv::imshow("HSV", HSV);
		cv::imshow("frame", frame);

		
		cv::waitKey(100);
	}
	Camera.release();
	
}

void CPi::vision_cal()
{
	//this part was copied from an online example
	/*cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
	//cv::Canny(edges, edges, 0, 30, 3);
	*/
	H_box.clear();
	S_box.clear();
	V_box.clear();

	cv::Rect recthsv((frame.size().width / 2) - 25, (frame.size().height / 2) - 25, 50, 50);
	cv::rectangle(frame, recthsv, cv::Scalar(0, 255, 0));
	for (int i = recthsv.x; i<recthsv.x + recthsv.width; i++)
	{
		//iterate through both x and y direction and save HSV values at each and every point
		for (int j = recthsv.y; j<recthsv.y + recthsv.height; j++)
		{
			//save HSV value at this point
			H_box.push_back((int)HSV.at<cv::Vec3b>(j, i)[0]);
			S_box.push_back((int)HSV.at<cv::Vec3b>(j, i)[1]);
			V_box.push_back((int)HSV.at<cv::Vec3b>(j, i)[2]);
		}
	}

	if (H_box.size()>0)
	{
		//NOTE: min_element and max_element return iterators so we must dereference them with "*"
		H_MIN = *std::min_element(H_box.begin(), H_box.end());
		H_MAX = *std::max_element(H_box.begin(), H_box.end());

	}
	if (S_box.size()>0)
	{
		S_MIN = *std::min_element(S_box.begin(), S_box.end());
		S_MAX = *std::max_element(S_box.begin(), S_box.end());

	}
	if (V_box.size()>0)
	{
		V_MIN = *std::min_element(V_box.begin(), V_box.end());
		V_MAX = *std::max_element(V_box.begin(), V_box.end());

	}
std::cout << "\r" << "MIN 'H':" << std::left << std::setw(5) << H_MIN
			<< "MAX 'H':" << std::setw(5) << H_MAX
			<< "MIN 'S':" << std::setw(5) << S_MIN
			<< "MAX 'S':" << std::setw(5) << S_MAX
			<< "MIN 'V':" << std::setw(5) << V_MIN
			<< "MAX 'V':" << std::setw(5) << V_MAX;
	//end of copied part

	//cv::imshow("edges", edges);
}
void CPi::vision_go()
{
	//threshold matrix
	inRange(HSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), threshold);
	/*radius.clear();
	midmass.clear();
	*/
	cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));

	cv::erode(threshold, threshold, erodeElement);
	cv::erode(threshold, threshold, erodeElement);


	cv::dilate(threshold, threshold, dilateElement);
	cv::dilate(threshold, threshold, dilateElement);


	findContours(threshold, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int iterate = contours.size();
	if (iterate > 0 && iterate <50)
	{

		std::vector <cv::Point2f> midmass(iterate);
		std::vector <float>  radius(iterate);

		for (unsigned int i = 0; i < iterate; i++)
		{


			//cv :: Rect obj_rec = boundingRect(contours.at(i));
			drawContours(frame, contours, i, cv::Scalar(255, 255, 255), cv::FILLED, 8, hierarchy);

			cv::minEnclosingCircle(contours[i], midmass[i], radius[i]);


		}


		auto radlong = std::max_element(radius.begin(), radius.end());
		int index = std::distance(std::begin(radius), radlong);
		//_mutex.lock();
		objpoint = midmass[index];
		//_mutex.unlock();
		objrad = radius[index];
		cv::circle(frame, objpoint, objrad, cv::Scalar(200, 200, 200));
	}



}
//
//void CPi::vision_thread(CPi* ptr)
//{
//	
//
//	while (ptr->_thread_exit == false)
//	{
//		ptr->vision();
//	}
//	
//	return;
//}

void CPi::command()
{
	while (_thread_exit == false)
	{
		char comand;
		std::cout << "press q to quit\n ";
		std::cin >> comand;
		_thread_exit = true;
	}
}
//
//void CPi::command_thread(CPi* ptr)
//{
//	while (ptr->_thread_exit == false)
//	{
//		ptr->vision();
//	}
//
//	return;
//}
//
//void CPi::start()
//{
//	std::thread t1(&CPi::command_thread, this);
//	t1.detach();
//	std::thread t2(&CPi::vision_thread, this);
//	t2.detach();
//}
//void CPi::run()
//{
//	start();
//	while (_thread_exit == false)
//	{
//		cv::waitKey(1000);
//		std :: cout << "got here/n";
//	}
//	cv::waitKey(1000);
//
//}
