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
#include <vector>

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
	if(N==0) return boost::numeric::ublas::vector<T>(0);
	const int n = N-1;

	boost::numeric::ublas::vector<T> vec(P[0].size());
	for(unsigned int i=0; i<vec.size(); ++i) vec[i] = T(0);

	for(int i=0; i<=n; ++i)
		vec += P[i]*bernstein(i, n, t);

	return vec;
}


// see:
// http://mathworld.wolfram.com/B-Spline.html
template<typename T> T bspline_base(int i, int j, T t, const std::vector<T>& knots)
{
	if(j==0)
	{
		if((knots[i] <= t) && (t < knots[i+1]) && (knots[i]<knots[i+1]))
			return 1.;
		return 0.;
	}

	T val11 = (t - knots[i]) / (knots[i+j]-knots[i]);
	T val12 = bspline_base(i, j-1, t, knots);
	T val1 = val11 * val12;

	T val21 = (knots[i+j+1]-t) / (knots[i+j+1]-knots[i+1]);
	T val22 = bspline_base(i+1, j-1, t, knots);
	T val2 = val21 * val22;

	T val = val1 + val2;
	return val;
}


// see:
// http://mathworld.wolfram.com/B-Spline.html
template<typename T>
boost::numeric::ublas::vector<T> bspline(const boost::numeric::ublas::vector<T>* P, unsigned int N, T t, const std::vector<T>& knots)
{
	if(N==0) return boost::numeric::ublas::vector<T>(0);
	const int n = N-1;
	const int m = knots.size()-1;
	const int degree = m-n-1;

	boost::numeric::ublas::vector<T> vec(P[0].size());
	for(unsigned int i=0; i<vec.size(); ++i) vec[i] = T(0);

	for(int i=0; i<=n; ++i)
		vec += P[i]*bspline_base(i, degree, t, knots);

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


class BSpline : public FunctionModel_param
{
	protected:
		boost::numeric::ublas::vector<double> *m_pvecs;
		unsigned int m_iN, m_iDegree;
		std::vector<double> m_vecKnots;

	public:
		BSpline(unsigned int N, const double *px, const double *py, unsigned int iDegree=3);
		virtual ~BSpline();

		virtual boost::numeric::ublas::vector<double> operator()(double t) const;
};

#endif
