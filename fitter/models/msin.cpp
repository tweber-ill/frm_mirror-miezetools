/*
 * A MIEZE signal fitter for the oscillation monitors using Minuit2
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#include <math.h>
#include <limits>
#include <algorithm>
#include <boost/algorithm/minmax_element.hpp>
#include <sstream>

#include <Minuit2/FCNBase.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnPrint.h>

#include "msin.h"
#include "../chi2.h"
#include "../../helper/misc.h"
#include "../../helper/fourier.h"


//----------------------------------------------------------------------
// sin function model with fixed frequency

MiezeSinModel::MiezeSinModel(double dFreq, double dAmp,
							 double dPhase, double dOffs,
							 double dFreqErr, double dAmpErr,
							 double dPhaseErr, double dOffsErr)
	: m_dfreq(dFreq), m_damp(dAmp), m_dphase(dPhase), m_doffs(dOffs),
	  m_dfreqerr(dFreqErr), m_damperr(dAmpErr),
	  m_dphaseerr(dPhaseErr), m_doffserr(dOffsErr)
{}

MiezeSinModel::~MiezeSinModel()
{}

std::string MiezeSinModel::print(bool bFillInSyms) const
{
	std::ostringstream ostr;
	if(bFillInSyms)
		ostr << m_damp << "*sin(" << m_dfreq << "*x + " << m_dphase << ") + " << m_doffs;
	else
		ostr << "amp * sin(freq*x + phase) + offs";
	return ostr.str();
}

bool MiezeSinModel::SetParams(const std::vector<double>& vecParams)
{
	m_damp = vecParams[0];
	m_dphase = vecParams[1];
	m_doffs = vecParams[2];

	if(m_damp < 0.)
		return false;

	return true;
}

double MiezeSinModel::operator()(double x) const
{
	return m_damp*sin(m_dfreq*x + m_dphase) + m_doffs;
}

FunctionModel* MiezeSinModel::copy() const
{
	return new MiezeSinModel(m_dfreq, m_damp, m_dphase, m_doffs,
							m_dfreqerr, m_damperr, m_dphaseerr, m_doffserr);
}

double MiezeSinModel::GetContrast() const
{
	return m_damp / m_doffs;
}

double MiezeSinModel::GetContrastErr() const
{

	// Gaussian error propagation
	// C = A/O
	// dC = dA/O - A/(O^2)*dO
	double dErr = sqrt((1/m_doffs*m_damperr)*(1/m_doffs*m_damperr)
				+ (-m_damp/(m_doffs*m_doffs)*m_doffserr)*(-m_damp/(m_doffs*m_doffs)*m_doffserr));

	
	return dErr;
}

bool get_mieze_contrast(double& dFreq, double& dNumOsc, unsigned int iLen,
					const double* px, const double* py, const double *pdy,
					MiezeSinModel** pmodel)
{
	if(dNumOsc<0.)
	{
		if(iFitterVerbosity >= 2)
			std::cerr << "Warning: No number of oscillations given, assuming 2 oscillations."
						<< std::endl;
		dNumOsc = 2.;
	}

	double* pdx_predef = 0;
	if(dFreq < 0.)
	{
		dFreq = 2.*M_PI/double(iLen) * dNumOsc;

		if(iFitterVerbosity >= 2)
			std::cerr << "Warning: No frequency given, calculating using number of bins."
				  	  << std::endl;

		px = 0;
	}

	if(!px)
	{
		pdx_predef = new double[iLen];
		for(unsigned int iIdx=0; iIdx<iLen; ++iIdx)
			pdx_predef[iIdx] = iIdx;

		px = pdx_predef;

		if(iFitterVerbosity >= 2)
			std::cerr << "Warning: Using predefined x values."
				  	  	  << std::endl;
	}

	MiezeSinModel sinmod(dFreq);
	Chi2Function fkt(&sinmod, iLen, px, py, pdy);


	typedef std::pair<const double*, const double*> t_minmax;
	t_minmax minmax_y = boost::minmax_element(py, py+iLen);

	const double dMin = *minmax_y.first;
	const double dMax = *minmax_y.second;

	if(dMax==dMin)
	{
		if(iFitterVerbosity >= 1)
			std::cerr << "Error: min == max, won't try fitting!" << std::endl;
		return 0;
	}

	// hints
	double dPhase = 0.;
	double dContrast_tmp = 0.;

	Fourier fourier(iLen);
	fourier.get_contrast(dNumOsc, py, dContrast_tmp, dPhase);

	// shift phase half a bin for correct alignment with mcstas data
	dPhase -= 0.5/double(iLen) * 2.*M_PI * dNumOsc;

	dPhase = fmod(dPhase, 2.*M_PI);
	if(dPhase > M_PI)
		dPhase -= 2.*M_PI;

	double dAmp = 0.5*(dMax - dMin);
	double dOffs = dMin + dAmp;

	//std::cerr << "hints: amp=" << dAmp << ", phase=" << dPhase << ", offs=" << dOffs << std::endl;

	ROOT::Minuit2::MnUserParameters params;
	params.Add("amp", dAmp, 0.1*dAmp);
	params.Add("phase", dPhase, 0.1*M_PI);
	params.Add("offs", dOffs, 0.1*dOffs);

	params.SetLimits("amp", 0., dMax);
	params.SetLimits("phase", -M_PI, M_PI);
	params.SetLimits("offs", dMin, dMax);

	bool bValidFit = false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;
	minis.reserve(4);

	{
		// first step: get phase
		params.Fix("amp");
		params.Fix("offs");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/ 0);
		//migrad.SetPrecision(std::numeric_limits<double>::epsilon());
		ROOT::Minuit2::FunctionMinimum mini = migrad();

		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("phase", mini.UserState().Value("phase"));
			params.SetError("phase", mini.UserState().Error("phase"));
		}

		minis.push_back(mini);
	}

	{
		// second step: get amp & offs
		params.Release("amp");
		params.Release("offs");
		params.Fix("phase");

		ROOT::Minuit2::MnMigrad migrad2(fkt, params, /*MINUIT_STRATEGY*/ 0);
		ROOT::Minuit2::FunctionMinimum mini2 = migrad2();

		bValidFit = mini2.IsValid() && mini2.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("amp", mini2.UserState().Value("amp"));
			params.SetError("amp", mini2.UserState().Error("amp"));

			params.SetValue("offs", mini2.UserState().Value("offs"));
			params.SetError("offs", mini2.UserState().Error("offs"));
		}

		minis.push_back(mini2);
	}


	{
		// third step: free fit (limited)
		params.Release("phase");

		ROOT::Minuit2::MnMigrad migrad3(fkt, params, /*MINUIT_STRATEGY*/1);
		ROOT::Minuit2::FunctionMinimum mini3 = migrad3();
		bValidFit = mini3.IsValid() && mini3.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("amp", mini3.UserState().Value("amp"));
			params.SetError("amp", mini3.UserState().Error("amp"));

			params.SetValue("offs", mini3.UserState().Value("offs"));
			params.SetError("offs", mini3.UserState().Error("offs"));

			params.SetValue("phase", mini3.UserState().Value("phase"));
			params.SetError("phase", mini3.UserState().Error("phase"));
		}

		minis.push_back(mini3);
	}


	{
		// fourth step: free fit (unlimited)
		params.RemoveLimits("amp");
		params.RemoveLimits("offs");
		params.RemoveLimits("phase");

		ROOT::Minuit2::MnMigrad migrad4(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini4 = migrad4();
		bValidFit = mini4.IsValid() && mini4.HasValidParameters();

		minis.push_back(mini4);
	}

	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();

	dAmp = lastmini.UserState().Value("amp");
	dPhase = lastmini.UserState().Value("phase");
	dOffs = lastmini.UserState().Value("offs");

	double dAmpErr = lastmini.UserState().Error("amp");
	double dPhaseErr = lastmini.UserState().Error("phase");
	double dOffsErr = lastmini.UserState().Error("offs");

	dAmpErr = fabs(dAmpErr);
	dPhaseErr = fabs(dPhaseErr);
	dOffsErr = fabs(dOffsErr);

	if(dAmp < 0.)
	{
		dAmp = -dAmp;
		dPhase += M_PI;
	}

	dPhase = fmod(dPhase, 2.*M_PI);
	if(dPhase<0.)
		dPhase += 2.*M_PI;

	if(iFitterVerbosity >= 3)
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			std::cerr << "result of MIEZE sinus fit step " << (++uiMini) << std::endl;
			std::cerr << "================================" << std::endl;
			std::cerr << mini << std::endl;
		}

		std::cerr << "values max: " << dMax << ", min: " << dMin << ", nchan=" << iLen << std::endl;
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}

	*pmodel = new MiezeSinModel(dFreq, dAmp, dPhase, dOffs,
								0., dAmpErr, dPhaseErr, dOffsErr);
	
	double dContrast = (*pmodel)->GetContrast();
	double dContrastError = (*pmodel)->GetContrastErr();

	if (has_nan_or_inf(dContrast) || has_nan_or_inf(dContrastError) ||
		has_nan_or_inf(dPhase) || has_nan_or_inf(dPhaseErr))
		bValidFit = 0;
	
	if(pdx_predef) delete[] pdx_predef;

	return bValidFit;
}
