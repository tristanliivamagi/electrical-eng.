#pragma once
#include "Spring.h"

#include <iostream>
#include <math.h>
#include <iomanip>

#include <string>

#define lo_added  0.015
#define Pi  3.141592654
#define Dred_lo  0.146 + lo_added
#define Dred_k  280.41
#define Dred_fcs 12.18
#define Dgreen_lo 0.150 + 0.015 
#define Dgreen_k  264.84
#define Dgreen_fcs  4.9064
#define Dblue_lo  0.152 + 0.015
#define Dblue_k  266.51
#define Dblue_fcs  7.3506
class CNewmeth
{

//private:
public:


	/*const double Dred_lo = 0.146 + lo_added, Dred_k=280.41, Dred_fcs =12.18;
	const double Dgreen_lo =0.150 + 0.015, Dgreen_k = 264.84, Dgreen_fcs = 4.9064;
	const double Dblue_lo = 0.152 + 0.015, Dblue_k = 266.51, Dblue_fcs = 7.3506;
*//*
	double red_lo, red_k , red_fcs ;
	double green_lo  , green_k , green_fcs ;
	double blue_lo , blue_k , blue_fcs ;*/

	void equation();
	
	void Get_guess();
	
	void user();
	
	
	CSpring red_sp; //spring object for red
	CSpring green_sp;//spring object for green
	CSpring blue_sp;//spring object for blue
	
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

