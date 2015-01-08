// tw
// aug-2012

#include <iostream>
#include <math.h>

double bragg_lam(double d, double tt)
{
	return 2.*d * sin(tt/2.);
}

double bragg_tt(double d, double lam, double n=1.)
{
	return 2. * asin(n*lam / (2.*d));
}

double lam2k(double lam)
{
	return 2.*M_PI / lam;
}

double theta(double d, double lam)
{
	return asin(0.5*lam/d);
}

int main()
{
	double dDet = 0.2;
	double dSampleDet = 1.05;
	double dDetAngleRange = atan(dDet/dSampleDet)/M_PI*180.;

	double dMono = 3.355;
	double dSample = 5.78;
	//double dSample = 22.98;

	//std::cout << bragg_tt(dSample, 4.8, 1.)/M_PI*180. << std::endl;
	//std::cout << bragg_tt(dSample, 4.8, 2.)/M_PI*180. << std::endl;


	double dPeakPos = 45. + dDetAngleRange/2. - 15.2/128.*dDetAngleRange;
	double dLam = bragg_lam(dSample, dPeakPos/180.*M_PI);

	std::cout << "lam: " << dLam << std::endl;
	std::cout << "k: " << lam2k(dLam) << std::endl;
	std::cout << "mono theta: " << theta(dMono, dLam)/M_PI*180. << std::endl;

	return 0;
}
