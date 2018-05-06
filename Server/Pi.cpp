//#include "stdafx.h"
#include "Pi.h"


CPi::CPi()
{


	Camera.open();
	
	green.colorid('g');
	red.colorid('r');

}


CPi::~CPi()
{
	Camera.release();
}
void CPi::vision()
{

	
	while (true)
	{
		Camera.grab();
		Camera.retrieve(frame);

		
	
		//green.vision_cal(frame);

		
		
	

		//red.vision_cal(frame);
		int xpos = red.vision_go(frame);
		std :: cout << "\n" << xpos << "\n" ;
		

	
		cv::imshow("frame", frame);
		
		cv::waitKey (10);
	}

	
}
