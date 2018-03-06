#include "stdafx.h"
#include "Print.h"

CPrint::CPrint()
{}

void CPrint::Get_values()
{
	int x = 0;
	while (x = 0) {
		cout << "Please Enter Ks, Lo, Fcs" << endl; // get input values from user
		cin >> Ks >> Lo >> Fcs;
		if (Ks > 0 && Lo > 0 && Fcs > 0) // Error checking for positive values
		{
			x = 1;
		}
		cout << "Invaid point. Please enter a positve value";
	}
}

void CPrint::Get_Coordinates()
{
	
	/*int x = 0;
	while (x = 0) 
	{*/
		cout << "Please Enter i, j Coordinates" << endl; // get inout coordinates from user
		int tempi, tempj;
		cin >> tempi >> tempj; 
		cout << endl <<"i Coordinate is: " <<tempi << endl << "j Coordinate is: "<<tempj << endl << endl;
		//if (tempi >= 0 && tempi <= 23 && tempj >= 0 && tempj <= 23) // Error Checking for a whole numbr point from 0 - 23
		//{
		//	x = 1;
		//}
		//cout << "Invaid point. Please enter a whole value point between 0 and 23";
		i = ((tempi * 2.54) / 100.0);  // Converting inches to meters
		j = ((tempj * 2.54) / 100.0);  // Converting inches to meters
	//}
	
}

double CPrint::Print_Menu(double angle, double L, double Ts, double POS)
{
	cout << "Angle"
		<< "	"
		<< "Lenght"
		<< "	"
		<< "Postition of Equilibruim"
		<< endl
		<< "  "
		<< angle
		<< " 	  "
		<< L
		<< "		   "
		<< POS;
	return 6;

}


CPrint::~CPrint()
{
}
