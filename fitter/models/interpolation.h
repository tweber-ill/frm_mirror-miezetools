/*
 * data point interpolation
 *
 * @author: Tobias Weber
 * @date: 25-04-2013
 */

#ifndef __MIEZE_INTERPOLATION__
#define __MIEZE_INTERPOLATION__

#include "../fitter.h"
#include <math.h>
#include <boost/math/special_functions/binomial.hpp>

// see:
// http://mathworld.wolfram.com/BernsteinPolynomial.html
template<typename T> T bernstein(int i, int n, T t)
{
	T bino = boost::math::binomial_coefficient<T>(n, i);
	return bino * pow(t, i) * pow(1-t, n-i);
}

// see:
// http://mathworld.wolfram.com/BezierCurve.html
template<typename T>
boost::numeric::ublas::vector<T> bezier(const boost::numeric::ublas::vector<T>* P, unsigned int N, T t)
{
	int n = N-1;

	boost::numeric::ublas::vector<T> vec(P[0].size());
	for(unsigned int i=0; i<vec.size(); ++i) vec[i] = T(0);

	for(int i=0; i<=n; ++i)
		vec += P[i]*bernstein(i, n, t);

	return vec;
}




class Bezier : public FunctionModel_param
{
	protected:
		boost::numeric::ublas::vector<double> *m_pvecs;
		unsigned int m_iN;

	public:
		Bezier(unsigned int N, const double *px, const double *py);
		virtual ~Bezier();

		virtual boost::numeric::ublas::vector<double> operator()(double t) const;
};

/*
class BSpline : public FunctionModel_param
{
	protected:
		boost::numeric::ublas::vector<double> *m_pvecs;
		unsigned int m_iN;

	public:
		BSpline(unsigned int N, const double *px, const double *py);
		virtual ~BSpline();

		virtual boost::numeric::ublas::vector<double> operator()(double t) const;
};*/

#endif
