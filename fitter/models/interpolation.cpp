/*
 * data point interpolation
 *
 * @author: Tobias Weber
 * @date: 25-04-2013
 */

#include "interpolation.h"
//#include <boost/algorithm/minmax_element.hpp>

namespace ublas = boost::numeric::ublas;


Bezier::Bezier(unsigned int N, const double *px, const double *py)
	: m_pvecs(0), m_iN(N)
{
	m_pvecs = new ublas::vector<double>[m_iN];

	for(unsigned int i=0; i<N; ++i)
	{
		m_pvecs[i].resize(2);
		m_pvecs[i][0] = px[i];
		m_pvecs[i][1] = py[i];
	}

	//auto MinMax = boost::minmax_element(px, px+N);
	//m_dMin = *MinMax.first;
	//m_dMax = *MinMax.second;
}

Bezier::~Bezier()
{
	if(m_pvecs) delete[] m_pvecs;
}


ublas::vector<double> Bezier::operator()(double t) const
{
	return ::bezier<double>(m_pvecs, m_iN, t);
}
