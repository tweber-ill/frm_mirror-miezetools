/*
 * A gauss n-dim fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: July 2012
 */

// TODO: normalize

#include "gauss-nd.h"

#include "../chi2.h"

#include <limits>
#include <algorithm>
#include <sstream>

#include <Minuit2/FCNBase.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnPrint.h>
#include <Minuit2/MnMinos.h>


#include <boost/algorithm/minmax_element.hpp>

/*
 * component-wise min-max of an array,
 * only use value with index i if pbUseVal[i]==true
 */
template<typename T> static void get_minmax(unsigned int uiLen, const T *pE,
                   T& tMinE, T& tMaxE,
                   const bool *pbUseVal=0,
                   unsigned int *piMinIdx=0, unsigned int *piMaxIdx=0)
{
        if(piMinIdx) *piMinIdx = 0;
        if(piMaxIdx) *piMaxIdx = 0;

        tMinE = std::numeric_limits<T>::max();
        tMaxE = -tMinE;

        for(unsigned int i=0; i<uiLen; ++i)
        {
                if(pbUseVal && !pbUseVal[i])
                        continue;

                if(pE[i] < tMinE)
                {
                        tMinE = pE[i];
                        if(piMinIdx) *piMinIdx = i;
                }

                if(pE[i] > tMaxE)
                {
                        tMaxE = pE[i];
                        if(piMaxIdx) *piMaxIdx = i;
                }

                //tMinE = std::min(tMinE, pE[i]);
                //tMaxE = std::max(tMaxE, pE[i]);
        }
}


GaussModel_nd::GaussModel_nd(unsigned int uiDim, double amp, const double* pspread, const double* px0)
{
	m_uiDim = uiDim;

	m_amp = amp;
	m_pspread = new double[uiDim];
	m_px0 = new double[uiDim];

	for(unsigned int i=0; i<m_uiDim; ++i)
	{
		m_pspread[i] = pspread ? pspread[i] : 0.;
		m_px0[i] = px0 ? px0[i] : 0.;
	}
}

GaussModel_nd::~GaussModel_nd()
{
	if(m_pspread) { delete[] m_pspread; m_pspread = 0; }
	if(m_px0) { delete[] m_px0; m_px0 = 0; }
}

unsigned int GaussModel_nd::GetDim() const
{
	return m_uiDim;
}

FunctionModel_nd* GaussModel_nd::copy() const
{
	return new GaussModel_nd(m_uiDim, m_amp, m_pspread, m_px0);
}

std::string GaussModel_nd::print() const
{
	const char* pcVar[] = {"x", "y", "z", "w"};
	
	std::ostringstream ostr;
	ostr << m_amp;
	
	for(unsigned int i=0; i<m_uiDim; ++i)
	{
		ostr << " * exp(-0.5 * ((" << pcVar[i] <<  "-" << m_px0[i] << ")/"
			 << m_pspread[i] << ")**2)";
	}

	return ostr.str();
}

bool GaussModel_nd::SetParams(const std::vector<double>& vecParams)
{
	m_amp = vecParams[0];
	
	for(unsigned int i=0; i<m_uiDim; ++i)
	{
		m_pspread[i] = vecParams[0*m_uiDim + i + 1];
		m_px0[i] = vecParams[1*m_uiDim + i + 1];
	}

	return 1;
}

double GaussModel_nd::operator()(const double* px) const
{
	double dval = m_amp;

	for(unsigned int i=0; i<m_uiDim; ++i)
	{
		double x = px[i];
		dval *= exp(-0.5 * ((x-m_px0[i])/m_pspread[i])*((x-m_px0[i])/m_pspread[i]));
	}

	return dval;
}


int get_gauss_nd(unsigned int uiDim, unsigned int iLen,
				const double **ppx, const double *py, const double *pdy,
				double& dAmp, double *pSpread, double *pX0,
				double& dAmp_err, double *pSpread_err, double *pX0_err)
{
	GaussModel_nd gmod(uiDim);
	Chi2Function_nd fkt(&gmod, iLen, ppx, py, pdy);


	double *pdXMin = new double[uiDim];
	double *pdXMax = new double[uiDim];	
	for(unsigned int i=0; i<uiDim; ++i)
	{
		get_minmax<double>(iLen, ppx[i], pdXMin[i], pdXMax[i]);
		//std::cout << "min=" << pdXMin[i] << ", max=" << pdXMax[i] <<std::endl;
	}
	

	double dMin, dMax;
	get_minmax<double>(iLen, py, dMin, dMax);
	//std::cout << "min=" << dMin << ", max=" << dMax <<std::endl;	
	
	//dAmp = dMax;
	//dAmp_err = 0.1*dMax;


	ROOT::Minuit2::MnUserParameters params;
	params.Add("amp", dAmp, dAmp_err);
	
	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostr;
		ostr << "spread_" << i;
		params.Add(ostr.str(), pSpread[i], pSpread_err[i]);
		
		//std::cout << "spread " << i << ": " << pSpread[i] << "+-" << pSpread_err[i] << std::endl;
	}

	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostr;
		ostr << "x0_" << i;
		params.Add(ostr.str(), pX0[i], pX0_err[i]);

		//std::cout << "x0 " << i << ": " << pX0[i] << "+-" << pX0_err[i] << std::endl;
	}


	// step 1: find spread
	params.SetLimits("amp", 0., dMax-dMin);
	params.Fix("amp");

	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrSpread, ostrx0;
		ostrSpread << "spread_" << i;
		ostrx0 << "x0_" << i;

		params.SetLowerLimit(ostrSpread.str(), 0.);
		params.SetLimits(ostrx0.str(), pdXMin[i], pdXMax[i]);
		params.Fix(ostrx0.str());		
	}



	ROOT::Minuit2::MnMigrad *migrad = new ROOT::Minuit2::MnMigrad(fkt, params, 0);
	ROOT::Minuit2::FunctionMinimum *mini = new ROOT::Minuit2::FunctionMinimum((*migrad)());
	//std::cout << (*mini) << std::endl;
	
	bool bValidFit = mini->IsValid() && mini->HasValidParameters();
	
	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrSpread;
		ostrSpread << "spread_" << i;

		params.SetValue(ostrSpread.str(), mini->UserState().Value(ostrSpread.str()));
		params.SetError(ostrSpread.str(), mini->UserState().Error(ostrSpread.str()));
	}




	// step 2 (free fit, limited)
	delete mini;
	delete migrad;

	params.Release("amp");
	
	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrx0;
		ostrx0 << "x0_" << i;

		params.Release(ostrx0.str());
	}	

	migrad = new ROOT::Minuit2::MnMigrad(fkt, params, 1);
	mini = new ROOT::Minuit2::FunctionMinimum((*migrad)());

	bValidFit = mini->IsValid() && mini->HasValidParameters();

	params.SetValue("amp", mini->UserState().Value("amp"));
	params.SetError("amp", mini->UserState().Error("amp"));
	
	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrSpread, ostrx0;
		ostrSpread << "spread_" << i;
		ostrx0 << "x0_" << i;

		params.SetValue(ostrSpread.str(), mini->UserState().Value(ostrSpread.str()));
		params.SetError(ostrSpread.str(), mini->UserState().Error(ostrSpread.str()));
		params.SetValue(ostrx0.str(), mini->UserState().Value(ostrx0.str()));
		params.SetError(ostrx0.str(), mini->UserState().Error(ostrx0.str()));
	}



	// step 3 (free fit, unlimited)
	delete mini;
	delete migrad;

	params.RemoveLimits("amp");

	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrSpread, ostrx0;
		ostrSpread << "spread_" << i;
		ostrx0 << "x0_" << i;

		params.RemoveLimits(ostrSpread.str());
		params.RemoveLimits(ostrx0.str());
	}


	migrad = new ROOT::Minuit2::MnMigrad(fkt, params, 2);
	mini = new ROOT::Minuit2::FunctionMinimum((*migrad)());

	bValidFit = mini->IsValid() && mini->HasValidParameters();

	/*
	if(bValidFit)
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		std::cerr << "result of fit" << std::endl;
		std::cerr << "=============" << std::endl;
		std::cerr << (*mini) << std::endl;

		ROOT::Minuit2::MnMinos minos(fkt, *mini);
		for(unsigned int iParam=0; iParam<params.Params().size(); ++iParam)
		{
			std::pair<double, double> err = minos(iParam);
			
			std::cerr << "minos error for " << params.GetName(iParam) << ": "
						<< err.first << ", " << err.second << std::endl;
		}
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}*/


	dAmp = mini->UserState().Value("amp");
	dAmp_err = mini->UserState().Error("amp");

	for(unsigned int i=0; i<uiDim; ++i)
	{
		std::ostringstream ostrSpread, ostrx0;
		ostrSpread << "spread_" << i;
		ostrx0 << "x0_" << i;

		pSpread[i] = mini->UserState().Value(ostrSpread.str());
		pSpread_err[i] = mini->UserState().Error(ostrSpread.str());
		pX0[i] = mini->UserState().Value(ostrx0.str());
		pX0_err[i] = mini->UserState().Error(ostrx0.str());
	}	
	
		
	delete mini;
	delete migrad;

	return bValidFit;
}
