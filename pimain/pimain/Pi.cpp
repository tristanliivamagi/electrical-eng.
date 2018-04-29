#include "stdafx.h"
#include "Pi.h"


CPi::CPi()
{

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

unsigned int CPi::vision_thread(CPi* ptr)
{
	while (ptr->_thread_exit == false)
	{
		ptr->vision();
	}

	return 0;
}

void CPi::command()
{
	char comand;
	std::cout << "press q to quit\n ";
	std :: cin >> comand;
	_thread_exit = true;

}

unsigned int CPi::command_thread(CPi* ptr)
{
	while (ptr->_thread_exit == false)
	{
		ptr->vision();
	}

	return 0;
}
void CPi::start()
{

	std::thread t2(&CPi::vision_thread, this);
	t2.detach();
	std::thread t1(&CPi::command_thread, this);
	t1.detach();
}