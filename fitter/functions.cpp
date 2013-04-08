/*
 * only some wrapper for boost's special functions
 *
 * Author: tweber@frm2.tum.de
 * Date: April 2012
 */

#include "functions.h"
#include <sys/time.h>


boost::mt19937 g_randgen;

void init_special_functions()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	g_randgen.seed(tv.tv_usec);
}

template<> double my_randab<double>(double, double);
template<> double my_rand01<double>();

#include <boost/math/special_functions/asinh.hpp>
#include <boost/math/special_functions/acosh.hpp>
#include <boost/math/special_functions/atanh.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/math/special_functions/erf.hpp>

double my_asinh(double x) { return boost::math::asinh(x); }
double my_acosh(double x) { return boost::math::acosh(x); }
double my_atanh(double x) { return boost::math::atanh(x); }
double my_round(double x) { return boost::math::round(x); }
double my_sign(double x) { return double(boost::math::sign(x)); }
double my_erf(double x) { return boost::math::erf(x); }
double my_erf_inv(double x) { return boost::math::erf_inv(x); }
