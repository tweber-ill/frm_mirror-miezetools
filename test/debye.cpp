/*
 * Debye-Waller-Factor
 */

#include <boost/units/io.hpp>
#include <fstream>
#include "../helper/neutrons.hpp"

typedef units::quantity<units::si::length> length;
typedef units::quantity<units::si::temperature> temp;
typedef units::quantity<units::si::mass> mass;
typedef units::quantity<units::si::wavenumber> wavenum;

int main()
{
	const temp kelvin = 1. * units::si::kelvin;
	const length A = 1e-10 * units::si::meter;
	const mass amu = 1.66e-27 * units::si::kilogram;

	temp T_D = 88. * kelvin;
	temp T = 5. * kelvin;
	wavenum Q = 1./A;
	mass M = 207. * amu;

	std::ofstream ofstr("debye.dat");

	for(Q=0./A; Q<10./A; Q+=0.5/A)
	{
		double dwf = debye_waller_low_T(T_D, T, M, Q);
		ofstr << double(Q*A) << " " << dwf << "\n";
	}

	return 0;
}
