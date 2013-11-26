/*
 * Bragg peaks for powder diffraction
 * @author tweber
 */

#include <iostream>
#include <cmath>


template<typename INT=int> bool is_even(INT i) { return (i%2 == 0); }
template<typename INT=int> bool is_odd(INT i) { return !is_even<INT>(i); }


bool is_peak_allowed_sc(int ih, int ik, int il) { return true; }

bool is_peak_allowed_fcc(int ih, int ik, int il) 
{ return ((is_even(ih) && is_even(ik) && is_even(il)) || (is_odd(ih) && is_odd(ik) && is_odd(il))); }

bool is_peak_allowed_bcc(int ih, int ik, int il) { return is_even(ih + ik + il); }


double calc_d(int ih, int ik, int il, double da)
{
	return da/std::sqrt(double(ih*ih) + double(ik*ik) + double(il*il));
}


// n lam = 2d sin th
// asin(n lam / (2d)) = th
double get_bragg_angle(int ih, int ik, int il, double dLam, double da)
{
        double d = calc_d(ih, ik, il, da);
        //std::cout << d << std::endl;
        double dTwotheta = std::asin(dLam / (2.*d)) * 2.;
        return dTwotheta;
}


int main(int argc, char** argv)
{
	const double da = 3.52;
	const double dLam = 1.;

	bool (*is_allowed[])(int, int, int) = {is_peak_allowed_sc, is_peak_allowed_fcc, is_peak_allowed_bcc};
	std::string strStr[] = {"sc", "fcc", "bcc"};

	for(int iStr=0; iStr<3; ++iStr)
	{
		std::cout << strStr[iStr] << "\n";

	        for(int ih=0; ih<4; ++ih) for(int ik=0; ik<4; ++ik) for(int il=0; il<4; ++il)
        	{
			if(ih==0 && ik==0 && il ==0) continue;
			if(!is_allowed[iStr](ih, ik, il)) continue;

			double dAngle = get_bragg_angle(ih, ik, il, dLam, da);

			std::cout << "(" << ih << ik << il << "): " << "twotheta = " << dAngle/M_PI*180. << "\n";
	        }

		std::cout << "\n\n";
	}
}
