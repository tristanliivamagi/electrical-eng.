
#include "stdafx.h"
#include "Spring.h"




CSpring::~CSpring()
{
}

CSpring::CSpring()
{

}
double CSpring::radtodeg(double rad)
{
	return rad * 180 / Pi ;

}
void CSpring::set_color(std :: string col)
{
	color = col;

}

double getdouble()
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
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.sync(); // clear the input buffer
	return temp;
}

void CSpring::Get_values()
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
int getint()
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
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std :: cin.sync(); // clear the input buffer
	return temp;
}
void CSpring::Get_Coordinates()
{

	int tempi = 0, tempj = 0;
	std :: cout << "\n\nFor The " << color 
		<< " Spring Please Enter i Coordinate:\n" ; // get inout coordinates from user
	tempi = getint();
	
	std ::cout << "\nPlease Enter j Coordinate:\n";
	tempj = getint();
	
	std :: cout << std::setw(7) << "\ni Coordinate is: " << tempi << "\nj Coordinate is: " << tempj << "\n\n";
	
	i_s = ((tempi * 2.54) / 100.0);  // Converting inches to meters
	j_s = ((tempj * 2.54) / 100.0);  // Converting inches to meters
								   

}

void CSpring::Print_values()
{
	lamb_s = Fcs - (Ks * Lo);
	std :: cout << std::setw(8) << "\n\nSpring constants For "<<color
		<<  "\nspring Ks: " << Ks
		<< "Lo : " << Lo 
		<< "  Fcs: " << Fcs
		<< "  lam_s"<<lamb_s;

}

void CSpring::Print_results()
{
	std::cout << std::setw(8) << "\n\nFor "<< color <<"spring the angle is : "
		<< radtodeg(angle)
		<< "degrees spring length is : "
		<< L
		<<"\n ";
	
	

}
double CSpring::der_phiy_yc()
{
	
	double temp = ((lamb_s * (pow(sin(angle), 2))) - Ts) / Ls;
	
	return temp;
}

double CSpring::der_phix_yc()
{
	
	double temp = (lamb_s / Ls) * cos(angle) * sin(angle);
	
	return temp;
}

double CSpring::der_phix_xc()
{
	
	double temp = ((lamb_s * (pow(cos(angle), 2))) - Ts) / Ls;
	
	return temp;
}

double CSpring::der_phiy_xc()
{
	
	double temp = (lamb_s / Ls) * cos(angle) * sin(angle);
	
	return temp;
}
////// Will return force magnitude and angle. /////// 
void CSpring::Force( double i_c, double j_c)
{
	//double Ls; // Spring Lenght
	lamb_s = Fcs - (Ks * Lo);
	Ls = sqrt( pow((i_s - i_c),2) + pow((j_s - j_c),2) ); // calculate lenght of loaded spring
	//angle = (i_s - i_c) / Ls; // calculate angle of loaded spring
	double y = (j_s - j_c);
	double x = (i_s - i_c);
	angle = atan2( y , x )  ;
	if (Lo > Ls)
	{
		lo_ok = false;
		std::cout << std::setw(7) << "\n for " << color << " possible error Lo > Ls \n";
	}
	else
	{
		lo_ok = true;
	}

	 Ts = Fcs + (Ks * (Ls - Lo));
	 i_Force = Ts * cos(angle);
	 j_Force = Ts * sin(angle);
	
}


