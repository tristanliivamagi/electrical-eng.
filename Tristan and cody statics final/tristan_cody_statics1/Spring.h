#pragma once
#include <iostream>	


#include <string>
#include "conio.h"
#include <math.h>
//#include<stdio.h>
#include<iostream>
#include <iomanip>
#include<ios>     // for <streamsize>
#include<limits>  // for numeric_limits

#define Pi  3.141592654
class CSpring
{

public:
	~CSpring();
	CSpring();

	double radtodeg(double rad);
	std :: string color="failure to initialize";
	void set_color(std :: string color);
	void Get_values();
	void Get_Coordinates();
	void Force(double i_c, double j_c);	 // Calculate Force 

	void Print_values();
	void Print_results();

	double der_phiy_yc();
	double der_phix_yc();
	double der_phix_xc();
	double der_phiy_xc();
	bool lo_ok;


	double i_s, j_s;
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

