
#pragma once
#include <iostream>	
#include "stdafx.h"

class CSpring
{

public:



	void spring_constants(double lo, double k, double fcs);
	~CSpring();
	CSpring();
	
	double CSpring::der_phiy_yc();
	double CSpring::der_phix_yc();
	double CSpring::der_phix_xc();
	double CSpring::der_phiy_xc();
	
	void Force(double i_s, double j_s, double i_c, double j_c);	 // Calculate Force 

	double i_Force , j_Force;
	double lamb_s;
     // Variables // 
	double  Ks;		// Spring Constant
	double Lo;		// Initail Unloaded Spring lenght 
	double Ls;		// Spring Lenght
	double L;		// Lenght of loaded spring
	double Fcs;		// Initail preloaded spring force
	double angle;
	double Ts;		// Force required to stretch spring 
};

