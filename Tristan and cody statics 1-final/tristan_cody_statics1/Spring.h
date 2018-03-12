////////////////////////////////////////////////////////////////
// MECH 4630 statics and dynamics assignment 1
//the purpose of this program is to find the center point of 
//three springs using 
//balenced force equations and newtons method.
// Created Feb 23 by Tristan Liivamagi and Cody Bessey
// Last updated March 11, 2018
////////////////////////////////////////////////////////////////
//Class Spring produces the output values of a spring based on
//the inputs of spring constants and coordinates of the bolt and
//the spring intersection.
//it also prints out the spring constant values and the force
//components and angle of the spring
////////////////////////////////////////////////////////////////




#pragma once
#include <iostream>	
#include <string>
#include "conio.h"
#include <math.h>
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
	//input radians outputs degrees
	double radtodeg(double rad);
	//recieves color string for use throughout the class
	std :: string color="failure to initialize";

	void set_color(std :: string color);//the color of the spring

	void Get_values();//reads in and sets the spring constants from the user

	void Get_Coordinates();//reads in the coordinets of the bolt for the spring

	void Force(double i_c, double j_c);	 // Calculate Force 

	void Print_values();//prints out the spring constants to the console

	void Print_results();//prints out the angle and length of the spring

	double der_phiy_yc();//return dsphiy/dyc
	double der_phix_yc();//return dsphix/dyc
	double der_phix_xc();//return dsphix/dxc
	double der_phiy_xc();//return dsphiy/dxc

	bool lo_ok;//true if 
	double i_s, j_s;//bolt coordinets
	double i_Force , j_Force;//force vector
	double lamb_s;//derived spring constant
     // Variables // 
	double  Ks;		// Spring Constant
	double Lo;		// Initail Unloaded Spring lenght 
	double Ls;		// Spring Length
	double Fcs;		// Initail preloaded spring force
	double angle; //spring angle
	double Ts;		// Force required to stretch spring 
};

