


#pragma once

#include "conio.h"
#include "stdafx.h"
#include <iostream>	
using namespace std;

class CPrint
{
public:
	CPrint();
	~CPrint();
	void Get_values();
	void Get_Coordinates();
	double Print_Menu(double angle, double L, double Ts, double POS);

	double Ks;		// Spring Constant
	double Lo;		// Initail Unloaded Spring lenght 
	double L;		// Lenght of loaded spring
	double Fcs;		// Initail preloaded spring force
	double Ts;		// Force required to stretch spring 
	double i;		// x coordinate of spring
	double j;		// y coordinate of spring 
	double Angle;

};

