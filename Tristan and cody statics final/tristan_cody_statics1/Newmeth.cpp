
#include "stdafx.h"
#include "Newmeth.h"


CNewmeth::CNewmeth()
{
	

}

CNewmeth::~CNewmeth()
{
}

void CNewmeth::user()
{

	red_sp.set_color("red  ");
	green_sp.set_color("green");
	blue_sp.set_color("blue ");

	
	red_sp.Lo = Dred_lo, red_sp.Ks = Dred_k, red_sp.Fcs = Dred_fcs;
	green_sp.Lo = Dgreen_lo, green_sp.Ks = Dgreen_k, green_sp.Fcs = Dgreen_fcs;
	blue_sp.Lo = Dblue_lo, blue_sp.Ks = Dblue_k, blue_sp.Fcs = Dblue_fcs;
	bool quiting = false;
	while (quiting == false)
	{
		bool exit_menu = false;
		while (exit_menu == false)
		{
			std::cout << "\n\n\n___Previous spring constants___\n";
			red_sp.Print_values();
			green_sp.Print_values();
			blue_sp.Print_values();
			std :: cout << "\n\nPress \n'D'efault spring constants\n'N'ew spring constants\n'P'revious spring constants\n'Q'uit\n";
			char selection;
			std :: cin >> selection;
			switch (selection)
			{
			case 'd':
			case 'D':
				red_sp.Lo = Dred_lo, red_sp.Ks = Dred_k, red_sp.Fcs = Dred_fcs;
				green_sp.Lo = Dgreen_lo, green_sp.Ks = Dgreen_k, green_sp.Fcs = Dgreen_fcs;
				blue_sp.Lo = Dblue_lo, blue_sp.Ks = Dblue_k, blue_sp.Fcs = Dblue_fcs;
		
				red_sp.Print_values();
				green_sp.Print_values();
				blue_sp.Print_values();
				exit_menu = true;
				break;
			case 'n':
			case 'N':
				red_sp.Get_values();
				red_sp.Print_values();

				green_sp.Get_values();
				green_sp.Print_values();

				blue_sp.Get_values();
				blue_sp.Print_values();

				exit_menu = true;
				break;
			case 'p':
			case 'P':
				red_sp.Print_values();
				green_sp.Print_values();
				blue_sp.Print_values();
				exit_menu = true;
				break;
			case 'q':
			case 'Q':
				quiting = true;
				exit_menu = true;
				break;
			default:
				continue;
			}

		}
		if (quiting == false)
		{
			Get_guess();
			
			x_cm = i_guess;
			y_cm = j_guess;

			equation();
			if (red_sp.lo_ok == false || blue_sp.lo_ok == false || green_sp.lo_ok == false)
			{
				std::cout << "\n !! An error in your input values produced an impossible Lo > Ls !!\n";
				red_sp.Print_values();
				green_sp.Print_values();
				blue_sp.Print_values();
			}
			else
			{
				std::cout << "\n_______Final results with no errors_______\n";
			}
			std::cout << std::setw(8) << "\nCenter point x,y is: (" << x_cm << " , " << y_cm
				<< ") m \n\n";

			red_sp.Print_results();
			green_sp.Print_results();
			blue_sp.Print_results();

		}
	}
}

void CNewmeth::equation()
{
	std::cout << "\n\n\n___Start of newtons method___\n";
	double x_cmd, y_cmd;
	int count = 20;
	do
	{
		red_sp.Force( x_cm, y_cm);
		blue_sp.Force( x_cm, y_cm);
		green_sp.Force( x_cm, y_cm);

		sum_der_phix_xc = red_sp.der_phix_xc() + blue_sp.der_phix_xc() + green_sp.der_phix_xc();
		
		sum_der_phix_yc = red_sp.der_phix_yc() + blue_sp.der_phix_yc() + green_sp.der_phix_yc();
		
		sum_der_phiy_xc = red_sp.der_phiy_xc() + blue_sp.der_phiy_xc() + green_sp.der_phiy_xc();
		
		sum_der_phiy_yc = red_sp.der_phiy_yc() + blue_sp.der_phiy_yc() + green_sp.der_phiy_yc();
		
		sum_Fx = red_sp.i_Force + blue_sp.i_Force + green_sp.i_Force;
		sum_Fy = red_sp.j_Force + blue_sp.j_Force + green_sp.j_Force;

		x_cmd = (((sum_Fx * sum_der_phiy_yc) - (sum_Fy * sum_der_phix_yc)) /
			((sum_der_phix_xc * sum_der_phiy_yc) - (sum_der_phiy_xc * sum_der_phix_yc)));
		
		y_cmd = (((sum_Fy * sum_der_phix_xc) - (sum_Fx * sum_der_phiy_xc)) /
			((sum_der_phix_xc * sum_der_phiy_yc) - (sum_der_phiy_xc * sum_der_phix_yc)));
		

		x_cm = x_cm - x_cmd;
		std::cout << "\n x_cmd " << x_cmd;
		y_cm = y_cm - y_cmd;
		std::cout << "\n y_cmd " << y_cmd;


		std::cout << std::setw(8) << "\nCenter point x,y is: (" << x_cm << " , " << y_cm
			<< ") meters   force x,y is: (" << sum_Fx << " , " << sum_Fy << ") Newtons \n";
		red_sp.Print_results();
		green_sp.Print_results();
		blue_sp.Print_results();
		count--;

	} while (((fabs(x_cmd) > 0.00001 || fabs(y_cmd) > 0.00001)) && count > 0);
	std::cout << "\n___End of newtons method___\n\n\n";
}


void CNewmeth::Get_guess()
{
	
	red_sp.Get_Coordinates();

	
	green_sp.Get_Coordinates();
	
	
	blue_sp.Get_Coordinates();

	

	i_guess = ((red_sp.i_s + blue_sp.i_s + green_sp.i_s) / 3);
	j_guess = ((red_sp.j_s + blue_sp.j_s + green_sp.j_s) / 3);
	std::cout << "\n i_guess j_guess:" << i_guess << "  " << j_guess<<"\n";
}

