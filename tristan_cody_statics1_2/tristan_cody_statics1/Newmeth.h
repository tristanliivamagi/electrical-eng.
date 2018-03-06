#pragma once
#include <iostream>
#include <cmath>
#include <iomanip>
#include "Spring.h"
#include "Print.h"
using namespace std;
class CNewmeth
{

//private:
public:
	double red_lo = 0.146, red_k=28.04, red_fcs =1.218;
	double green_lo =0.150 , green_k = 26.48, green_fcs = 0.4916;
	double blue_lo = 0.152, blue_k = 26.65, blue_fcs = 0.7351;


	void equation();
	
	void Get_guess();
	void sumdiff();
	void user();
	

	CPrint red_pr;
	CPrint blue_pr;
	CPrint green_pr;

	CSpring red_sp;
	CSpring blue_sp;
	CSpring green_sp;
	double sum_der_phix_xc, sum_der_phiy_yc;
	double sum_der_phix_yc, sum_der_phiy_xc;
	double sum_Fx, sum_Fy;
	double x_cm;
	double y_cm;
	double i_guess;
	double j_guess;
	//bool red.Spring();
	CNewmeth();
	~CNewmeth();
	
	


};

