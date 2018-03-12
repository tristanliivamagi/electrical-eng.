////////////////////////////////////////////////////////////////
// MECH 4630 statics and dynamics assignment 1
//the purpose of this program is to find the center point of 
//three springs using 
//balenced force equations and newtons method.
// Created Feb 23 by Tristan Liivamagi and Cody Bessey
// Last updated March 11, 2018
////////////////////////////////////////////////////////////////
//Class Newmeth provides a user interface to input spring
//constants and coordinates for three springs
//on a 23 x 23 inch punch board
//and using newtons method outputs the resultant coordinate
//of the spring intersection
////////////////////////////////////////////////////////////////
#pragma once
#include "Spring.h"

#include <iostream>
#include <math.h>
#include <iomanip>
#include <string>

#define Pi  3.141592654//Pi

//spring constants for board 006
#define lo_added  0.015//the added distance of the bolt and the ring
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
public:
	
	
	void user();//menu selection for user 
	void equation();//newtons method

	//gets coordinets from the user and calculate the average as a
	//starting point for newtons method
	void Get_guess();
	CSpring red_sp; //spring object for red
	CSpring green_sp;//spring object for green
	CSpring blue_sp;//spring object for blue
	
	double sum_der_phix_xc, sum_der_phiy_yc;//sums of patial derivatives
	double sum_der_phix_yc, sum_der_phiy_xc;
	double sum_Fx, sum_Fy;//sum of forces
	double x_cm;//centerspring junction x coordinates
	double y_cm;//centerspring junction y coordinates
	double i_guess;//initial average of bolt possitions
	double j_guess;//initial average of bolt possitions
	//bool red.Spring();
	CNewmeth();
	~CNewmeth();
	
	


};

