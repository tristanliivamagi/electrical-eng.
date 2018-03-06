#include "stdafx.h"
#include <iostream>	
#include "Spring.h"
#include "conio.h"
#include <math.h>
#include<stdio.h>
#include "Print.h"
#include "Newmeth.h"


using namespace std;

void CSpring::spring_constants(double lo, double k, double fcs)
{
	Lo = lo;
	Ks = k;
	Fcs = fcs;
	lamb_s = Fcs - (Ks * Lo);
}

CSpring::~CSpring()
{
}

CSpring::CSpring()
{

}

double CSpring::der_phiy_yc()
{
	//return (((pow(sin(angle), 2) - 1) * lamb_s / Ls) - (2 * Ks * sin(angle)));
	return ((lamb_s * (pow(sin(angle), 2))) - Ts) / Ls;
}

double CSpring::der_phix_yc()
{
	//return ((lamb_s / Ls) * cos(angle) * sin(angle))-2*Ks*cos(angle);
	return (lamb_s / Ls) * cos(angle) * sin(angle);
}

double CSpring::der_phix_xc()
{
	//return (((pow(cos(angle), 2) - 1) * lamb_s / Ls) - (2 * Ks * cos(angle)));
	return ((lamb_s * (pow(cos(angle), 2))) - Ts) / Ls;
}

double CSpring::der_phiy_xc()
{
	//return ((lamb_s / Ls) * cos(angle) * sin(angle)) - 2 * Ks * sin(angle);
	return (lamb_s / Ls) * cos(angle) * sin(angle);
}
////// Will return force magnitude and angle. /////// 
void CSpring::Force( double i_s, double j_s, double i_c, double j_c)
{
	//double Ls; // Spring Lenght

	Ls = sqrt( pow((i_s - i_c),2) + pow((j_s - j_c),2) ); // calculate lenght of loaded spring
	//angle = (i_s - i_c) / Ls; // calculate angle of loaded spring
	double y = (j_s - j_c);
	double x = (i_s - i_c);
	angle = atan2( y , x )  ;
	if (Lo > Ls)
	{
		cout << "Inncorrect Lo"; 
	}

	 Ts = Fcs + (Ks * (Ls - Lo));
	 i_Force = Ts * cos(angle);
	 j_Force = Ts * sin(angle);

	
}


