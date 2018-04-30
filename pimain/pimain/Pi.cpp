//#include "stdafx.h"
#include "Pi.h"


CPi::CPi()
{
	_thread_exit=false;
}


CPi::~CPi()
{
}
void CPi::vision()
{
	raspicam::RaspiCam_Cv Camera;

	Camera.open();

	cv::Mat image;
	while (_thread_exit == false)
	{
		Camera.grab();
		Camera.retrieve(image);
		cv::imshow("asdf", image);
		cv::waitKey(100);
	}
	Camera.release();
	
}

void CPi::vision_thread(CPi* ptr)
{
	

	while (ptr->_thread_exit == false)
	{
		ptr->vision();
	}
	
	return;
}

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

void CPi::command_thread(CPi* ptr)
{
	while (ptr->_thread_exit == false)
	{
		ptr->vision();
	}

	return;
}
void CPi::start()
{
	std::thread t1(&CPi::command_thread, this);
	t1.detach();
	std::thread t2(&CPi::vision_thread, this);
	t2.detach();
}
void CPi::run()
{
	start();
	while (_thread_exit == false)
	{
		
	}
	cv::waitKey(1000);

}
