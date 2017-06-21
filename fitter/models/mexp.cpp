/**
 * spin-echo exponential function
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date June 2017
 * @license GPLv3
 */

#include "tlibs/math/math.h"
#include "tlibs/math/linalg.h"
#include "tlibs/log/log.h"

#include <limits>
#include <algorithm>
#include <boost/algorithm/minmax_element.hpp>
#include <sstream>
#include <fstream>

#include <Minuit2/FCNBase.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnPrint.h>

#include "mexp.h"
#include "../chi2.h"
#include "tlibs/helper/misc.h"
#include "tlibs/phys/units.h"


//----------------------------------------------------------------------

// hbar in mueV*ps
const double MiezeExpModel::s_dhbar = tl::co::hbar / tl::one_eV / tl::units::si::second * 1e18;


MiezeExpModel::MiezeExpModel(double dP0, double dGamma, double dP0Err, double dGammaErr)
	: m_dP0(dP0), m_dGamma(dGamma), m_dP0Err(dP0Err), m_dGammaErr(dGammaErr)
{}

MiezeExpModel::~MiezeExpModel()
{}

std::string MiezeExpModel::print(bool bFillInSyms) const
{
	std::ostringstream ostr;
	if(bFillInSyms)
		ostr << m_dP0 << " * exp(-" << m_dGamma << "*x / " << s_dhbar << ")";
	else
		ostr << "P0 * exp(-Gamma*x / " << s_dhbar << ")";
	return ostr.str();
}

bool MiezeExpModel::SetParams(const std::vector<double>& vecParams)
{
	m_dP0 = vecParams[0];
	m_dGamma = vecParams[1];

	return true;
}

double MiezeExpModel::operator()(double dTau) const
{
	return m_dP0 * std::exp(-m_dGamma * dTau / s_dhbar);
}

FunctionModel* MiezeExpModel::copy() const
{
	return new MiezeExpModel(m_dP0, m_dGamma, m_dP0Err, m_dGammaErr);
}

std::vector<std::string> MiezeExpModel::GetParamNames() const
{
	std::vector<std::string> m_vecNames = {"P0", "Gamma"};
	return m_vecNames;
}
std::vector<double> MiezeExpModel::GetParamValues() const
{
	std::vector<double> vecVals = {GetP0(), GetGamma() };
	return vecVals;
}
std::vector<double> MiezeExpModel::GetParamErrors() const
{
	std::vector<double> vecVals = {GetP0Err(), GetGammaErr() };
	return vecVals;
}

bool get_mieze_gamma(unsigned int iLen,
	const double* px, const double* py, const double *pdy,
	MiezeExpModel** pmodel, bool bFixP0)
{
	double* pdx_predef = 0;

	if(!px)
	{
		pdx_predef = new double[iLen];
		for(unsigned int iIdx=0; iIdx<iLen; ++iIdx)
			pdx_predef[iIdx] = iIdx;

		px = pdx_predef;

		tl::log_warn("Using predefined x values.");
	}

	MiezeExpModel expmod{};
	Chi2Function fkt(&expmod, iLen, px, py, pdy);


	typedef std::pair<const double*, const double*> t_minmax;
	t_minmax minmax_x = boost::minmax_element(px, px+iLen);

	double dMin = *minmax_x.first;
	double dMax = *minmax_x.second;
	if(dMax < dMin) std::swap(dMax, dMin);


	// hints
	double dP0 = 1.;
	double dGamma = MiezeExpModel::s_dhbar / (dMax-dMin);

	ROOT::Minuit2::MnUserParameters params;
	params.Add("P0", dP0, 0.25*dP0);
	params.Add("Gamma", dGamma, 0.25*dGamma);

	params.SetLimits("P0", 0., 1.);
	params.SetLowerLimit("Gamma", 0.);

	if(bFixP0)
	{
		params.SetValue("P0", 1.);
		params.SetError("P0", 0.);
		params.Fix("P0");
	}

	bool bValidFit = false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;
	minis.reserve(4);

	if(!bFixP0)
	{
		// get P0
		params.Fix("Gamma");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/ 0);
		//migrad.SetPrecision(std::numeric_limits<double>::epsilon());
		ROOT::Minuit2::FunctionMinimum mini = migrad();

		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("P0", mini.UserState().Value("P0"));
			params.SetError("P0", mini.UserState().Error("P0"));
		}

		minis.push_back(mini);
	}

	{
		// free fit (limited)
		params.Release("Gamma");

		ROOT::Minuit2::MnMigrad migrad3(fkt, params, /*MINUIT_STRATEGY*/1);
		ROOT::Minuit2::FunctionMinimum mini3 = migrad3();
		bValidFit = mini3.IsValid() && mini3.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("P0", mini3.UserState().Value("P0"));
			params.SetError("P0", mini3.UserState().Error("P0"));

			params.SetValue("Gamma", mini3.UserState().Value("Gamma"));
			params.SetError("Gamma", mini3.UserState().Error("Gamma"));
		}

		minis.push_back(mini3);
	}


	{
		// free fit (unlimited)
		params.RemoveLimits("P0");
		params.RemoveLimits("Gamma");

		ROOT::Minuit2::MnMigrad migrad4(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini4 = migrad4();
		bValidFit = mini4.IsValid() && mini4.HasValidParameters();

		minis.push_back(mini4);
	}


	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();

	dP0 = lastmini.UserState().Value("P0");
	dGamma = lastmini.UserState().Value("Gamma");

	double dP0Err = lastmini.UserState().Error("P0");
	double dGammaErr = lastmini.UserState().Error("Gamma");

	dP0Err = fabs(dP0Err);
	dGammaErr = fabs(dGammaErr);


	//if(iFitterVerbosity >= 3)
	{
		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			tl::log_info("result of MIEZE exponential fit step ", (++uiMini));
			std::ostringstream ostrMini; ostrMini << mini,
			tl::log_info(ostrMini.str());
		}
	}


	*pmodel = new MiezeExpModel(dP0, dGamma, dP0Err, dGammaErr);
	if(pdx_predef) delete[] pdx_predef;
	return bValidFit;
}
