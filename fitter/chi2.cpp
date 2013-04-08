/*
 * Chi^2 calculation
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#include <limits>
#include <math.h>

#include "chi2.h"
#include "../helper/misc.h"

// chi^2 calculation
// based on the example in the Minuit user's guide:
// http://seal.cern.ch/documents/minuit/mnusersguide.pdf
double Chi2Function::chi2(const std::vector<double>& vecParams) const
{
	// cannot operate on m_pfkt directly, because Minuit
	// uses more than one thread!
	FunctionModel* pfkt = m_pfkt->copy();
	autodeleter<FunctionModel> a(pfkt);

	bool bParamsOk = pfkt->SetParams(vecParams);
	//if(!bParamsOk)
	//	return std::numeric_limits<double>::max();

	//std::cout << (*m_pfkt) << std::endl;

	double dChi2 = 0.;

	for(unsigned int i=0; i<m_uiLen; ++i)
	{
		double d = (*pfkt)(m_px[i]) - m_py[i];
		double dy = m_pdy[i];
		if(fabs(dy) < std::numeric_limits<double>::min())
			dy = std::numeric_limits<double>::min();

		d /= dy;
		dChi2 += d*d;
	}
	return dChi2;
}

double Chi2Function_nd::chi2(const std::vector<double>& vecParams) const
{
	FunctionModel_nd* pfkt = m_pfkt->copy();
	autodeleter<FunctionModel_nd> _a0(pfkt);

	bool bParamsOk = pfkt->SetParams(vecParams);
	
	double *px = new double[m_uiDim];
	autodeleter<double> _a1(px, 1);
	
	double dChi2 = 0.;

	for(unsigned int i=0; i<m_uiLen; ++i)
	{
		for(unsigned int iX=0; iX<m_uiDim; ++iX)
			px[iX] = m_vecpx[iX][i];
		
		double d = (*pfkt)(px) - m_py[i];
		double dy = m_pdy[i];
		if(fabs(dy) < std::numeric_limits<double>::min())
			dy = std::numeric_limits<double>::min();

		d /= dy;
		dChi2 += d*d;
	}
	return dChi2;
}
