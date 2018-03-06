
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
	Get_guess();
	red_sp.spring_constants(red_lo, red_k, red_fcs);
	blue_sp.spring_constants(blue_lo, blue_k, blue_fcs);
	green_sp.spring_constants(green_lo, green_k, green_fcs);
	x_cm = i_guess;
	y_cm = j_guess;
	equation();

}
void CNewmeth::equation()
{
	
	for(int i=1;i<20;i++)
	{



	sumdiff();

	

	x_cm = x_cm - (((sum_Fx * sum_der_phiy_yc) - (sum_Fy * sum_der_phix_yc)) /
		((sum_der_phix_xc * sum_der_phiy_yc) - (sum_der_phiy_xc * sum_der_phix_yc)));

	y_cm = y_cm - (((sum_Fy * sum_der_phix_xc) - (sum_Fx * sum_der_phiy_xc)) /
		((sum_der_phix_xc * sum_der_phiy_yc) - (sum_der_phiy_xc * sum_der_phix_yc)));

	

	cout << "\nCenter point is: (" << x_cm  << " , " << y_cm  << ") cm \nf x is: (" << sum_Fx << " , " << sum_Fy << ") Newtons \n";
	}
}
void CNewmeth::sumdiff()
{
	red_sp.Force(red_pr.i, red_pr.j, x_cm, y_cm);
	blue_sp.Force(blue_pr.i, blue_pr.j, x_cm, y_cm);
	green_sp.Force(green_pr.i, green_pr.j, x_cm, y_cm);

	sum_der_phix_xc = red_sp.der_phix_xc() + blue_sp.der_phix_xc() + green_sp.der_phix_xc();
	sum_der_phix_yc = red_sp.der_phix_yc() + blue_sp.der_phix_yc() + green_sp.der_phix_yc();
	sum_der_phiy_xc = red_sp.der_phiy_xc() + blue_sp.der_phiy_xc() + green_sp.der_phiy_xc();
	sum_der_phiy_yc = red_sp.der_phiy_yc() + blue_sp.der_phiy_yc() + green_sp.der_phiy_yc();
	sum_Fx = red_sp.i_Force + blue_sp.i_Force + green_sp.i_Force;
	sum_Fy = red_sp.j_Force + blue_sp.j_Force + green_sp.j_Force;

}


void CNewmeth::Get_guess()
{
	cout << "For the Red Spring ";
	red_pr.Get_Coordinates();
	
	cout << "For the Blue Spring";
	blue_pr.Get_Coordinates();

	cout << "For the Green Spring";
	green_pr.Get_Coordinates();

	i_guess = ((red_pr.i + blue_pr.i + green_pr.i) / 3);
	j_guess = ((red_pr.j + blue_pr.j + green_pr.j) / 3);
}

