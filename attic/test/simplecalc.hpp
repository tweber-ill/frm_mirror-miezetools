/**
 * stuff for quick neutron calculations
 * @author tweber
 * @date 11-sep-2014
 */

#ifndef __SIMPLECALC_H__
#define __SIMPLECALC_H__

#include "../helper/neutrons.hpp"
#include <boost/units/io.hpp>

using namespace units;

units::quantity<units::si::length, long double> operator "" _A(long double ldA)
{
	return ldA * 1e-10 * units::si::meter;
}

units::quantity<units::si::frequency, long double> operator "" _Hz(long double ldHz)
{
	return ldHz / units::si::second;
}

units::quantity<units::si::plane_angle, long double> operator "" _rad(long double ldAng)
{
	return ldAng * units::si::radians;
}

units::quantity<units::si::plane_angle, long double> operator "" _deg(long double ldAng)
{
	return ldAng / 180. * M_PI * units::si::radians;
}

units::quantity<units::si::energy, long double> operator "" _eV(long double ldE)
{
	double de = co::e / units::si::coulombs;
	return ldE * de * units::si::coulombs * units::si::volts;
}

units::quantity<units::si::energy, long double> operator "" _meV(long double ldE)
{
	double de = co::e / units::si::coulombs;
	return ldE * 1e-3 * de * units::si::coulombs * units::si::volts;
}


// --------------------------------------------------------------------------------


#include "../helper/linalg.h"
#include <vector>

using namespace ublas;

using vec = ublas::vector<long double, std::vector<long double>>;
using mat = ublas::matrix<long double>;

#endif
