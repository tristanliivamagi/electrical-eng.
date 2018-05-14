//#include "stdafx.h"
#include "Color.h"


CColor::CColor()
{
	
	//H_MIN = 56;
	//H_MAX = 79;
	//S_MIN = 108;
	//S_MAX = 146;
	//V_MIN = 104;
	//V_MAX = 133;

}


CColor::~CColor()
{
}

void CColor::colorid(char name)
{
	id = name;
	std::ostringstream oss;
	oss << id << ".txt" ;
	filename = oss.str();
	std :: cout << "\n" << filename << "\n" ;
	gethsv();
	std::cout << "\r" << "MIN 'H':" << std::left << std::setw(5) << H_MIN
		<< "MAX 'H':" << std::setw(5) << H_MAX
		<< "MIN 'S':" << std::setw(5) << S_MIN
		<< "MAX 'S':" << std::setw(5) << S_MAX
		<< "MIN 'V':" << std::setw(5) << V_MIN
		<< "MAX 'V':" << std::setw(5) << V_MAX;
}
void CColor::storehsv()
{
	
	std::ofstream myfile (filename);
	if (myfile.is_open())
	{
		myfile <<offseth<< " "<< H_MIN << " " << H_MAX<< "\n";
		myfile <<offsets<< " "<< S_MIN << " " << S_MAX<< "\n";
		myfile <<offsetv<< " "<< V_MIN << " " << V_MAX<< "\n";
		
		myfile.close();
	}
	else std :: cout << "\nunable to open file\n";
	
}
void CColor::gethsv()
{
	
	std::ifstream myfile (filename);
	if (myfile.is_open())
	{
		myfile >>offseth >> H_MIN >> H_MAX;
		myfile >>offsets >> S_MIN >> S_MAX;
		myfile >>offsetv >> V_MIN >> V_MAX;
		
		myfile.close();
	}
	else std :: cout << "\nunable to open file\n";
	H_MIN=H_MIN-offseth;
	if(H_MIN < hsvmin)
	{
		H_MIN = hmax + H_MIN;
	}
	H_MAX=H_MAX+offseth;
	if(H_MAX>hmax)
	{
		H_MAX = H_MAX-hmax;
	}
	S_MIN=S_MIN-offsets;
	if(S_MIN<hsvmin)
	{
		S_MIN=hsvmin;
	}
	S_MAX=S_MAX+offsets;
	if(S_MAX>svmax)
	{
		S_MAX=svmax;
	}
	V_MIN=V_MIN-offsetv;
	if(V_MIN<hsvmin)
	{
		V_MIN=hsvmin;
	}
	V_MAX=V_MAX+offsetv;
	if(V_MAX>svmax)
	{
		V_MAX=svmax;
	}
}
void CColor::vision_cal( cv::Mat myframe)
{
	//this part was copied from an online example
	//cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
	//cv::Canny(edges, edges, 0, 30, 3);
	
	cv::cvtColor(myframe, myHSV, cv::COLOR_RGB2HSV);
	H_box.clear();
	S_box.clear();
	V_box.clear();

	cv::Rect recthsv((myframe.size().width / 2) - 25, (myframe.size().height / 2) - 25, 50, 50);
	cv::rectangle(myframe, recthsv, cv::Scalar(0, 255, 0));
	for (int i = recthsv.x; i<recthsv.x + recthsv.width; i++)
	{
		//iterate through both x and y direction and save HSV values at each and every point
		for (int j = recthsv.y; j<recthsv.y + recthsv.height; j++)
		{
			//save HSV value at this point
			H_box.push_back((int)myHSV.at<cv::Vec3b>(j, i)[0]);
			S_box.push_back((int)myHSV.at<cv::Vec3b>(j, i)[1]);
			V_box.push_back((int)myHSV.at<cv::Vec3b>(j, i)[2]);
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
	storehsv();
	gethsv();

}


int CColor::vision_go(cv::Mat myframe)
{
	/*radius.clear();
	midmass.clear();
	*/
	//convert frame to HSV and put into Mat named HSV
	cv::cvtColor(myframe, myHSV, cv::COLOR_RGB2HSV);

	//threshold matrix
	if(H_MIN<=H_MAX)
	{
		inRange(myHSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), mythreshold);
	}
	else
	{
		std :: cout << "\nthis should not happen";
		inRange(myHSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(hmax, S_MAX, V_MAX), thresh1);
		inRange(myHSV, cv::Scalar(hsvmin, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), thresh1);
		mythreshold = thresh1 +thresh2;
		
	}
	


	//cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	//cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));

	//cv::erode(mythreshold, mythreshold, erodeElement);
	//cv::erode(mythreshold, mythreshold, erodeElement);


	//cv::dilate(mythreshold, mythreshold, dilateElement);
	//cv::dilate(mythreshold, mythreshold, dilateElement);

	int xpos;
	findContours(mythreshold, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	unsigned int iterate = contours.size();
	printf("number of contours %d\n", iterate);
	if (iterate > 0 && iterate <50)
	{

		std::vector <cv::Point2f> midmass(iterate);
		std::vector <float>  radius(iterate);

		for (unsigned int i = 0; i < iterate; i++)
		{


			//cv :: Rect obj_rec = boundingRect(contours.at(i));
			drawContours(myframe, contours, i, cv::Scalar(255, 255, 255), CV_FILLED, 8, hierarchy);

			cv::minEnclosingCircle(contours[i], midmass[i], radius[i]);


		}


		auto radlong = std::max_element(radius.begin(), radius.end());
		int index = std::distance(std::begin(radius), radlong);
		//_mutex.lock();
		objpoint = midmass[index];
		//std::cout << "\r" << objpoint;
		//_mutex.unlock();
		objrad = radius[index];
		cv::circle(myframe, objpoint, objrad, cv::Scalar(200, 200, 200));
		//xpos=( (200.0/  ( objpoint.x -  myframe.size().width  ) ) -100.0 );
		xpos= objpoint.x - (myframe.size().width/2);
		//distance= scaledist / objrad ;
	}
	else
	{
		if (iterate==0)
		{
			xpos=-1000;
		}
		else
		{
			xpos=1000;
		}
	}
	return xpos; 
	
}
