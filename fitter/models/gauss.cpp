/*
 * A gauss fitter using Minuit
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

#include "gauss.h"
#include "../chi2.h"

//----------------------------------------------------------------------
// see: http://mathworld.wolfram.com/GaussianFunction.html
static const double SIGMA2FWHM = 2.*sqrt(2.*log(2.));
static const double FWHM2SIGMA = 1./SIGMA2FWHM;
static const double SIGMA2HWHM = SIGMA2FWHM/2.;
static const double HWHM2SIGMA = 1./SIGMA2HWHM;
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// gauss model
GaussModel::GaussModel(double damp, double dspread, double dx0,
					   double damperr, double dspreaderr, double dx0err,
					   bool bNormalized)
			: m_amp(damp), m_spread(dspread), m_x0(dx0),
			  m_amperr(damperr), m_spreaderr(dspreaderr), m_x0err(dx0err),
			  m_bNormalized(bNormalized)
{}

GaussModel::~GaussModel()
{}

double GaussModel::GetMean() const { return m_x0; }
double GaussModel::GetSigma() const { return m_spread; }
double GaussModel::GetFWHM() const { return SIGMA2FWHM * m_spread; }
double GaussModel::GetHWHM() const { return SIGMA2HWHM * m_spread; }
double GaussModel::GetAmp() const
{
	if(!m_bNormalized)
		return m_amp;
	else
		return m_amp/sqrt(2.*M_PI*fabs(GetSigma()));
}

double GaussModel::GetMeanErr() const { return m_x0err; }
double GaussModel::GetSigmaErr() const { return m_spreaderr; }
double GaussModel::GetFWHMErr() const { return SIGMA2FWHM * m_spreaderr; }
double GaussModel::GetHWHMErr() const { return SIGMA2HWHM * m_spreaderr; }
double GaussModel::GetAmpErr() const { return m_amperr; }


std::string GaussModel::print(bool bFillInSyms) const
{
	std::ostringstream ostr;
	if(bFillInSyms)
	{
		ostr << m_amp;
		if(m_bNormalized)
			ostr << "/sqrt(2*pi * " << m_spread << ")";
		ostr << " * exp(-0.5 * ((x-" << m_x0 << ")/"
			 << m_spread << ")**2)";
	}
	else
	{
		ostr << "amp";
		if(m_bNormalized)
			ostr << "/sqrt(2*pi * sigma)";
		ostr << " * exp(-0.5 * ((x-x0) / sigma)**2)";
	}
	return ostr.str();
}

bool GaussModel::SetParams(const std::vector<double>& vecParams)
{
	m_amp = vecParams[0];
	m_spread = vecParams[1];
	m_x0 = vecParams[2];

	return true;
}

double GaussModel::operator()(double x) const
{
	double dNorm = 1.;
	if(m_bNormalized)
		dNorm = 1./(sqrt(2.*M_PI*fabs(m_spread)));
	
	return m_amp * dNorm
		* exp(-0.5 * ((x-m_x0)/m_spread)*((x-m_x0)/m_spread));
}

FunctionModel* GaussModel::copy() const
{
	return new GaussModel(m_amp, m_spread, m_x0,
							m_amperr, m_spreaderr, m_x0err,
							m_bNormalized);
}

void GaussModel::Normalize()
{
	if(!m_bNormalized)
	{
		// normalize
		// a' = a*sqrt(2*pi*s)
		// da' = da*sqrt(2*pi*s) + a*sqrt(2*pi) * dsqrt(s)
		// da' = da*sqrt(2*pi*s) + a*sqrt(2*pi) * 0.5*s^(-1/2)*ds
		m_amperr = sqrt(pow(m_amperr*sqrt(2.*M_PI*m_spread), 2.) +
				pow(m_amp*sqrt(2.*M_PI) * 0.5 * 1./sqrt(m_spread)*m_spreaderr, 2.));
		m_amp *= sqrt(2.*M_PI*fabs(m_spread));

		m_bNormalized = true;
	}
}


bool get_gauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					GaussModel **pmodel)
{
	GaussModel gmod;
	Chi2Function fkt(&gmod, iLen, px, py, pdy);

	typedef std::pair<const double*, const double*> t_minmax;
	t_minmax minmax_x = boost::minmax_element(px, px+iLen);
	t_minmax minmax_y = boost::minmax_element(py, py+iLen);

	const double dXMin = *minmax_x.first;
	const double dXMax = *minmax_x.second;

	const double *pdMax = minmax_y.second;
	const double dMin = *minmax_y.first;
	double dMax = *pdMax;
	int iMaxPos = int(pdMax-py);

	if(dMax==dMin)
	{
		std::cerr << "Error: min == max, won't try fitting!" << std::endl;
		return 0;
	}

	double dSpread = dXMax-dXMin;
	double dAmp = dMax;
	double dx0 = px[iMaxPos];

	ROOT::Minuit2::MnUserParameters params;
	params.Add("amp", dAmp, 0.5*dAmp);
	params.Add("spread", dSpread, (dXMax-dXMin)*0.5);
	params.Add("x0", dx0, 0.1*dx0);

	//params.SetLimits("amp", 0., dMax*(sqrt(2.*M_PI)*fabs(dSpread)));
	params.SetLowerLimit("spread", 0.);
	params.SetLimits("x0", dXMin, dXMax);

	bool bValidFit=false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;

	{
		// step 1: find amp & spread
		//params.Fix("amp");
		//params.Fix("spread");
		params.Fix("x0");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/0);
		ROOT::Minuit2::FunctionMinimum mini = migrad();

		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("spread", mini.UserState().Value("spread"));
			params.SetError("spread", mini.UserState().Error("spread"));

			params.SetValue("amp", mini.UserState().Value("amp"));
			params.SetError("amp", mini.UserState().Error("amp"));
		}

		minis.push_back(mini);
	}


	{
		// step 1.5: find amp
		params.Release("amp");
		params.Fix("spread");
		params.Fix("x0");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/0);
		ROOT::Minuit2::FunctionMinimum mini = migrad();

		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("amp", mini.UserState().Value("amp"));
			params.SetError("amp", mini.UserState().Error("amp"));
		}

		minis.push_back(mini);
	}

	{
		// step 2: free fit (limited)
		params.Release("amp");
		params.Release("spread");
		params.Release("x0");

		ROOT::Minuit2::MnMigrad migrad2(fkt, params, /*MINUIT_STRATEGY*/1);
		ROOT::Minuit2::FunctionMinimum mini2 = migrad2();
		bValidFit = mini2.IsValid() && mini2.HasValidParameters();

		//if(bValidFit)
		{
			params.SetValue("amp", mini2.UserState().Value("amp"));
			params.SetError("amp", mini2.UserState().Error("amp"));

			params.SetValue("spread", mini2.UserState().Value("spread"));
			params.SetError("spread", mini2.UserState().Error("spread"));

			params.SetValue("x0", mini2.UserState().Value("x0"));
			params.SetError("x0", mini2.UserState().Error("x0"));
		}

		minis.push_back(mini2);
	}


	{
		// step 3: free fit (unlimited)
		params.RemoveLimits("amp");
		params.RemoveLimits("spread");
		params.RemoveLimits("x0");

		ROOT::Minuit2::MnMigrad migrad3(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini3 = migrad3();
		bValidFit = mini3.IsValid() && mini3.HasValidParameters();

		minis.push_back(mini3);
	}


	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();


	dAmp = lastmini.UserState().Value("amp");
	dSpread = lastmini.UserState().Value("spread");
	dx0 = lastmini.UserState().Value("x0");

	double dAmpErr = lastmini.UserState().Error("amp");
	double dSpreadErr = lastmini.UserState().Error("spread");
	double dx0Err = lastmini.UserState().Error("x0");


	dSpread = fabs(dSpread);

	dAmpErr = fabs(dAmpErr);
	dSpreadErr = fabs(dSpreadErr);
	dx0Err = fabs(dx0Err);

	bool bNormalized = gmod.IsNormalized();

	/*
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			std::cerr << "result of gauss fit step " << (++uiMini) << std::endl;
			std::cerr << "==========================" << std::endl;
			std::cerr << mini << std::endl;
		}

		std::cerr << "values max: " << dMax << ", min: " << dMin << ", nchan=" << iLen << std::endl;
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}*/

	
	*pmodel = new GaussModel(dAmp, dSpread, dx0, dAmpErr, dSpreadErr, dx0Err, bNormalized);
	(*pmodel)->Normalize();

	return bValidFit;
}
