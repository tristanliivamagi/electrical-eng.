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

#include "stdafx.h"
#include "Spring.h"




CSpring::~CSpring()
{
}

CSpring::CSpring()
{

}
double CSpring::radtodeg(double rad)//input radians outputs degrees
{
	return rad * 180 / Pi ;

}
void CSpring::set_color(std :: string col)//recieves color string for use throughout the class
{
	color = col;

}

double getdouble() //error checking for the function below
{
	double temp;
	while (!(std::cin >> temp) ||  temp < 0)
	{
		//clears the fault flag
		std::cin.clear();

		// discards the input buffer
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "\nInvalid data type!\nplease enter a number greater than or equal to 0 : ";
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');// discards the input buffer
	std::cin.sync(); // clear the input buffer
	return temp;
}

void CSpring::Get_values()//reads in and sets the spring constants from the user
{
	
	std::cout << "\nFor The " << color
		<< " Spring Please Enter Ks Lo and Fcs spring Constants:\n"; // get spring constants from user

	std::cout << "\nKs :\n";
	Ks = getdouble();
	
	std::cout << "\nLo : ";
	Lo = getdouble();
	
	std::cout << "\nFcs : ";
	Fcs = getdouble();
	lamb_s = Fcs - (Ks * Lo);
	
}
int getint()//error checking for the function below
{
	int temp;
	while (!(std::cin >> temp) || temp>23 || temp < 0)
	{
		//clears the fault flag
		std::cin.clear();

		// discards the input buffer
		std :: cin.ignore(std :: numeric_limits<std :: streamsize>::max(), '\n');
		std::cout << "\nInvalid data type!\nplease enter an integer from 0 to 23 : ";
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');// discards the input buffer
	std :: cin.sync(); // clear the input buffer
	return temp;
}
void CSpring::Get_Coordinates()//reads in the coordinets of the bolt for the spring 
{

	int tempi = 0, tempj = 0;
	std :: cout << "\n\nFor The " << color 
		<< " Spring Please Enter i Coordinate:\n" ; 
	tempi = getint();// get i input coordinates from user
	
	std ::cout << "\nPlease Enter j Coordinate:\n";
	tempj = getint();// get j input coordinate from user
	
	std :: cout << std::setw(7) << "\ni Coordinate is: " << tempi 
		<< "\nj Coordinate is: " << tempj << "\n\n";
	
	i_s = ((tempi * 2.54) / 100.0);  // Converting inches to meters
	j_s = ((tempj * 2.54) / 100.0);  // Converting inches to meters
								   

}

void CSpring::Print_values()//prints out the spring constants to the console
{
	lamb_s = Fcs - (Ks * Lo);
	std :: cout  << "\n\nSpring constants For "<<color
		<< "\nspring Ks: " << std::setw(6) << Ks
		<< " Newtons/m   Lo : " << std::setw(6) << Lo
		<< " m   Fcs: " << std::setw(6) << Fcs
		<< " Newtons   lam_s: " << std::setw(6) << lamb_s
		<< " Newtons";
}

void CSpring::Print_results()//prints out the angle and length of the spring
{
	std::cout << std::setw(8) << "\n\nFor "<< color <<" spring the angle is : "
		<< std::setw(8) << radtodeg(angle)
		<< "  degrees spring length is : "
		<< std::setw(8) << Ls
		<<" meters\n ";
	
	

}
double CSpring::der_phiy_yc()//return dsphiy/dyc
{
	double temp = ((lamb_s * (pow(sin(angle), 2))) - Ts) / Ls;
	return temp;
}

double CSpring::der_phix_yc()//return dsphix/dyc
{
	double temp = (lamb_s / Ls) * cos(angle) * sin(angle);
	return temp;
}

double CSpring::der_phix_xc()//return dsphix/dxc
{
	double temp = ((lamb_s * (pow(cos(angle), 2))) - Ts) / Ls;
	return temp;
}

double CSpring::der_phiy_xc()//return dsphiy/dxc
{
	double temp = (lamb_s / Ls) * cos(angle) * sin(angle);
	return temp;
}
//calculates the spring tension and angle and set flag for Lo > Ls
void CSpring::Force( double i_c, double j_c)
{
	//double Ls; // Spring Lenght
	lamb_s = Fcs - (Ks * Lo);
	Ls = sqrt( pow((i_s - i_c),2) + pow((j_s - j_c),2) ); // calculate lenght of loaded spring
	double y = (j_s - j_c);
	double x = (i_s - i_c);
	angle = atan2( y , x )  ;// calculate angle of loaded spring
	if ((Lo > Ls) && Fcs>0.000001)//this condition should not happen for our springs
	{
		lo_ok = false;
		std::cout << "\n for " //error message
			<< color << " possible error Lo > Ls \n";
	}
	else
	{
		lo_ok = true;
	}

	 Ts = Fcs + (Ks * (Ls - Lo)); //calculates the spring tension
	 i_Force = Ts * cos(angle);//x component of spring tension
	 j_Force = Ts * sin(angle);//y component of spring tension
	
}


