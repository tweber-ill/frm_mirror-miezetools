/*
 * wrapper for boost's special functions
 *
 * Author: tweber@frm2.tum.de
 * Date: April 2012
 */

#ifndef __SPEC_FUNCTIONS__
#define __SPEC_FUNCTIONS__

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/math/special_functions/erf.hpp>

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>

#include <vector>

extern boost::mt19937 g_randgen;
void init_special_functions();

//------------------------------------------------------------------------------
// uniform distribution

template<typename T> 
T my_randab(T ta, T tb)
{
	typedef typename boost::mpl::if_<boost::is_floating_point<T>,
									boost::uniform_real<T>,
									boost::uniform_int<T> >
										::type t_dist;

	t_dist dist(ta, tb);
	boost::variate_generator<boost::mt19937&, t_dist >
			gen(g_randgen, dist);

	return gen();
}

// generates a random number between 0 and 1
template<typename T> T my_rand01()
{
	return my_randab<T>(0, 1);
}
//------------------------------------------------------------------------------


double my_acosh(double x);
double my_asinh(double x);
double my_atanh(double x);

double my_round(double x);
double my_sign(double x);

double my_erf(double x);
double my_erf_inv(double x);

#endif
