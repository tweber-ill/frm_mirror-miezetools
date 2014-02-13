/*
 * A gauss fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: April 2012, 25-apr-2013
 */

#include "../../helper/math.h"
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
#include "../../helper/misc.h"
#include "../../helper/math.h"
#include "interpolation.h"

#include "../../main/settings.h"


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
					   bool bNormalized,
					   double doffs, double doffserr)
			: m_amp(damp), m_spread(dspread), m_x0(dx0),
			  m_amperr(damperr), m_spreaderr(dspreaderr), m_x0err(dx0err),
			  m_offs(doffs), m_offserr(doffserr),
			  m_bNormalized(bNormalized)
{}

GaussModel::~GaussModel()
{}

double GaussModel::GetMean() const { return m_x0; }
double GaussModel::GetSigma() const { return m_spread; }
double GaussModel::GetFWHM() const { return SIGMA2FWHM * m_spread; }
double GaussModel::GetHWHM() const { return SIGMA2HWHM * m_spread; }
double GaussModel::GetOffs() const { return m_offs; }
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
double GaussModel::GetOffsErr() const { return m_offserr; }
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
			 << m_spread << ")^2) + " << m_offs;
	}
	else
	{
		ostr << "amp";
		if(m_bNormalized)
			ostr << "/sqrt(2*pi * sigma)";
		ostr << " * exp(-0.5 * ((x-x0) / sigma)^2) + offs";
	}
	return ostr.str();
}


bool GaussModel::SetParams(const std::vector<double>& vecParams)
{
	m_amp = vecParams[0];
	m_spread = vecParams[1];
	m_x0 = vecParams[2];
	m_offs = vecParams[3];

	return true;
}

double GaussModel::operator()(double x) const
{
	double dNorm = 1.;
	if(m_bNormalized)
		dNorm = 1./(sqrt(2.*M_PI*fabs(m_spread)));
	
	return m_amp * dNorm
		* exp(-0.5 * ((x-m_x0)/m_spread)*((x-m_x0)/m_spread)) + m_offs;
}

FunctionModel* GaussModel::copy() const
{
	return new GaussModel(m_amp, m_spread, m_x0,
							m_amperr, m_spreaderr, m_x0err,
							m_bNormalized, m_offs, m_offserr);
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

std::vector<std::string> GaussModel::GetParamNames() const
{
	std::vector<std::string> vecNames = {"amp", "sigma", "x0", "offs"};
	return vecNames;
}

std::vector<double> GaussModel::GetParamValues() const
{
	std::vector<double> vecVals = { m_amp, m_spread, m_x0, m_offs };
	return vecVals;
}

std::vector<double> GaussModel::GetParamErrors() const
{
	std::vector<double> vecErrs = { m_amperr, m_spreaderr, m_x0err, m_offserr };
	return vecErrs;
}




bool get_gauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					GaussModel **pmodel)
{
	std::vector<double> vecMaximaX;
	std::vector<double> vecMaximaSize;
	std::vector<double> vecMaximaWidth;

	const int iDegree = Settings::Get<int>("interpolation/spline_degree");
	find_peaks<double>(iLen, px, py, iDegree, vecMaximaX, vecMaximaSize, vecMaximaWidth);

	bool bPrefitOk = 1;
	if(vecMaximaX.size() < 1)
	{
		bPrefitOk = 0;
		vecMaximaX.resize(1);
	}
	if(vecMaximaSize.size() < 1)
		vecMaximaSize.resize(1);
	if(vecMaximaWidth.size() < 1)
		vecMaximaWidth.resize(1);


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
		if(iFitterVerbosity >= 1)
			std::cerr << "Error: min == max, won't try fitting!" << std::endl;
		return 0;
	}

	double dSpread = dXMax-dXMin;
	double dSpreadErr = 0.5*dSpread;

	double dAmp = dMax-dMin;
	double dAmpErr = 0.5*dAmp;

	double dx0 = px[iMaxPos];

	if(bPrefitOk)
	{
		dSpread = HWHM2SIGMA*vecMaximaWidth[0];
		dSpreadErr = 0.1*dSpread;

		dAmp = vecMaximaSize[0];
		dAmpErr = 0.1*dAmp;

		dx0 = vecMaximaX[0];
	}

	ROOT::Minuit2::MnUserParameters params;
	params.Add("amp", dAmp, dAmpErr);
	params.Add("spread", dSpread, dSpreadErr);
	params.Add("x0", dx0, 0.1*dx0);
	params.Add("offs", dMin, 0.1*dMin);

	//params.SetLimits("amp", 0., dMax*(sqrt(2.*M_PI)*fabs(dSpread)));
	params.SetLowerLimit("spread", 0.);
	params.SetLimits("x0", dXMin, dXMax);
	params.SetLimits("offs", dMin, dMax);

	bool bValidFit=false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;

	{
		// step 1: find amp & spread
		//params.Fix("amp");
		//params.Fix("spread");
		params.Fix("x0");
		params.Fix("offs");

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
		// step 1.5: find amp & offs
		params.Release("amp");
		params.Release("offs");
		params.Fix("spread");
		params.Fix("x0");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/0);
		ROOT::Minuit2::FunctionMinimum mini = migrad();

		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			params.SetValue("amp", mini.UserState().Value("amp"));
			params.SetError("amp", mini.UserState().Error("amp"));

			params.SetValue("offs", mini.UserState().Value("offs"));
			params.SetError("offs", mini.UserState().Error("offs"));
		}

		minis.push_back(mini);
	}

	{
		// step 2: free fit (limited)
		params.Release("amp");
		params.Release("spread");
		params.Release("x0");
		params.Release("offs");

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
		params.RemoveLimits("offs");

		ROOT::Minuit2::MnMigrad migrad3(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini3 = migrad3();
		bValidFit = mini3.IsValid() && mini3.HasValidParameters();

		minis.push_back(mini3);
	}


	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();


	dAmp = lastmini.UserState().Value("amp");
	dSpread = lastmini.UserState().Value("spread");
	dx0 = lastmini.UserState().Value("x0");
	double doffs =lastmini.UserState().Value("offs");

	dAmpErr = lastmini.UserState().Error("amp");
	dSpreadErr = lastmini.UserState().Error("spread");
	double dx0Err = lastmini.UserState().Error("x0");
	double doffserr =lastmini.UserState().Error("offs");


	dSpread = fabs(dSpread);

	dAmpErr = fabs(dAmpErr);
	dSpreadErr = fabs(dSpreadErr);
	dx0Err = fabs(dx0Err);

	bool bNormalized = gmod.IsNormalized();

	if(iFitterVerbosity >= 3)
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
	}

	
	*pmodel = new GaussModel(dAmp, dSpread, dx0, dAmpErr, dSpreadErr, dx0Err, bNormalized, doffs, doffserr);
	(*pmodel)->Normalize();

	return bValidFit;
}





//----------------------------------------------------------------------
// multi gauss model

MultiGaussModel::MultiGaussModel(unsigned int iNumGausses)
			: m_bNormalized(0), m_offs(0.), m_offserr(0.)
{
	this->m_vecParams.resize(iNumGausses);
}

MultiGaussModel::MultiGaussModel(const MultiGaussModel& model)
{
	this->operator=(model);
}

const MultiGaussModel& MultiGaussModel::operator=(const MultiGaussModel& model)
{
	this->m_bNormalized = model.m_bNormalized;
	this->m_vecParams = model.m_vecParams;
	this->m_offs = model.m_offs;
	this->m_offserr = model.m_offserr;

	return *this;
}

MultiGaussModel::~MultiGaussModel() {}

double MultiGaussModel::GetMean(unsigned int iIdx) const { return m_vecParams[iIdx].m_x0; }
double MultiGaussModel::GetSigma(unsigned int iIdx) const { return m_vecParams[iIdx].m_spread; }
double MultiGaussModel::GetFWHM(unsigned int iIdx) const { return SIGMA2FWHM * m_vecParams[iIdx].m_spread; }
double MultiGaussModel::GetHWHM(unsigned int iIdx) const { return SIGMA2HWHM * m_vecParams[iIdx].m_spread; }
double MultiGaussModel::GetOffs() const { return m_offs; }
double MultiGaussModel::GetAmp(unsigned int iIdx) const
{
	if(!m_bNormalized)
		return m_vecParams[iIdx].m_amp;
	else
		return m_vecParams[iIdx].m_amp/sqrt(2.*M_PI*fabs(GetSigma(iIdx)));
}

double MultiGaussModel::GetMeanErr(unsigned int iIdx) const { return m_vecParams[iIdx].m_x0err; }
double MultiGaussModel::GetSigmaErr(unsigned int iIdx) const { return m_vecParams[iIdx].m_spreaderr; }
double MultiGaussModel::GetFWHMErr(unsigned int iIdx) const { return SIGMA2FWHM * m_vecParams[iIdx].m_spreaderr; }
double MultiGaussModel::GetHWHMErr(unsigned int iIdx) const { return SIGMA2HWHM * m_vecParams[iIdx].m_spreaderr; }
double MultiGaussModel::GetOffsErr() const { return m_offserr; }
double MultiGaussModel::GetAmpErr(unsigned int iIdx) const { return m_vecParams[iIdx].m_amperr; }


std::string MultiGaussModel::print(bool bFillInSyms) const
{
	std::ostringstream ostr;
	if(bFillInSyms)
	{
		for(unsigned int i=0; i<m_vecParams.size(); ++i)
		{
			ostr << m_vecParams[i].m_amp;
			if(m_bNormalized)
				ostr << "/sqrt(2*pi * " << m_vecParams[i].m_spread << ")";
			ostr << " * exp(-0.5 * ((x-" << m_vecParams[i].m_x0 << ")/"
				 << m_vecParams[i].m_spread << ")^2) + ";
		}

		ostr << m_offs;
	}
	else
	{
		for(unsigned int i=0; i<m_vecParams.size(); ++i)
		{
			ostr << "amp_" << i;
			if(m_bNormalized)
				ostr << "/sqrt(2*pi * sigma_"<<i<<")";
			ostr << " * exp(-0.5 * ((x-x0_" <<i<<") / sigma_"<<i<<")^2) + ";
		}

		ostr << "offs";
	}
	return ostr.str();
}

std::vector<std::string> MultiGaussModel::GetParamNames() const
{
	std::vector<std::string> vecNames;

	for(unsigned int i=0; i<m_vecParams.size(); ++i)
	{
		std::ostringstream ostrAmp, ostrSigma, ostrX0;
		ostrAmp << "amp_" << i;
		ostrSigma << "sigma_" << i;
		ostrX0 << "x0_" << i;

		vecNames.push_back(ostrAmp.str());
		vecNames.push_back(ostrSigma.str());
		vecNames.push_back(ostrX0.str());
	}

	vecNames.push_back("offs");
	return vecNames;
}

std::vector<double> MultiGaussModel::GetParamValues() const
{
	std::vector<double> vecVals;
	for(unsigned int i=0; i<m_vecParams.size(); ++i)
	{
		vecVals.push_back(m_vecParams[i].m_amp);
		vecVals.push_back(m_vecParams[i].m_spread);
		vecVals.push_back(m_vecParams[i].m_x0);
	}
	vecVals.push_back(m_offs);

	return vecVals;
}

std::vector<double> MultiGaussModel::GetParamErrors() const
{
	std::vector<double> vecVals;
	for(unsigned int i=0; i<m_vecParams.size(); ++i)
	{
		vecVals.push_back(m_vecParams[i].m_amperr);
		vecVals.push_back(m_vecParams[i].m_spreaderr);
		vecVals.push_back(m_vecParams[i].m_x0err);
	}
	vecVals.push_back(m_offserr);

	return vecVals;
}


bool MultiGaussModel::SetParams(const std::vector<double>& vecParams)
{
	for(unsigned int i=0; i<m_vecParams.size(); ++i)
	{
		m_vecParams[i].m_amp = vecParams[i*3 + 0];
		m_vecParams[i].m_spread = vecParams[i*3 + 1];
		m_vecParams[i].m_x0 = vecParams[i*3 + 2];
	}
	m_offs = vecParams[vecParams.size()-1];

	return true;
}

double MultiGaussModel::operator()(double x) const
{
	double *dNorm = new double[m_vecParams.size()];

	if(m_bNormalized)
	{
		for(unsigned int i=0; i<m_vecParams.size(); ++i)
			dNorm[i] = 1./(sqrt(2.*M_PI*fabs(m_vecParams[i].m_spread)));
	}
	else
	{
		for(unsigned int i=0; i<m_vecParams.size(); ++i)
			dNorm[i] = 1.;
	}

	double dRes = 0.;

	for(unsigned int i=0; i<m_vecParams.size(); ++i)
		dRes += m_vecParams[i].m_amp * dNorm[i]
		                 * exp(-0.5 * ((x-m_vecParams[i].m_x0)/m_vecParams[i].m_spread)*((x-m_vecParams[i].m_x0)/m_vecParams[i].m_spread));

	dRes += m_offs;

	delete[] dNorm;
	return dRes;
}

FunctionModel* MultiGaussModel::copy() const
{
	return new MultiGaussModel(*this);
}

void MultiGaussModel::Normalize()
{
	if(!m_bNormalized)
	{
		// normalize
		// a' = a*sqrt(2*pi*s)
		// da' = da*sqrt(2*pi*s) + a*sqrt(2*pi) * dsqrt(s)
		// da' = da*sqrt(2*pi*s) + a*sqrt(2*pi) * 0.5*s^(-1/2)*ds
		for(unsigned int i=0; i<m_vecParams.size(); ++i)
		{
			m_vecParams[i].m_amperr = sqrt(pow(m_vecParams[i].m_amperr*sqrt(2.*M_PI*m_vecParams[i].m_spread), 2.) +
					pow(m_vecParams[i].m_amp*sqrt(2.*M_PI) * 0.5 * 1./sqrt(m_vecParams[i].m_spread)*m_vecParams[i].m_spreaderr, 2.));
			m_vecParams[i].m_amp *= sqrt(2.*M_PI*fabs(m_vecParams[i].m_spread));
		}

		m_bNormalized = true;
	}
}



bool get_multigauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					MultiGaussModel **pmodel, unsigned int iNumGauss)
{
	std::vector<double> vecMaximaX;
	std::vector<double> vecMaximaSize;
	std::vector<double> vecMaximaWidth;

	const int iDegree = Settings::Get<int>("interpolation/spline_degree");
	find_peaks<double>(iLen, px, py, iDegree, vecMaximaX, vecMaximaSize, vecMaximaWidth);

	if(vecMaximaX.size() < iNumGauss)
		vecMaximaX.resize(iNumGauss);
	if(vecMaximaSize.size() < iNumGauss)
		vecMaximaSize.resize(iNumGauss);
	if(vecMaximaWidth.size() < iNumGauss)
		vecMaximaWidth.resize(iNumGauss);


	MultiGaussModel gmod(iNumGauss);
	Chi2Function fkt(&gmod, iLen, px, py, pdy);

	typedef std::pair<const double*, const double*> t_minmax;
	//t_minmax minmax_x = boost::minmax_element(px, px+iLen);
	t_minmax minmax_y = boost::minmax_element(py, py+iLen);

	const double *pdMax = minmax_y.second;
	const double dMin = *minmax_y.first;
	double dMax = *pdMax;

	if(dMax==dMin)
	{
		if(iFitterVerbosity >= 1)
			std::cerr << "Error: min == max, won't try fitting!" << std::endl;
		return 0;
	}

	ROOT::Minuit2::MnUserParameters params;

	for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
	{
		std::ostringstream ostrAmp, ostrSpread, ostrX0;
		ostrAmp << "amp_" << iGauss;
		ostrSpread << "spread_" << iGauss;
		ostrX0 << "x0_" << iGauss;

		params.Add(ostrAmp.str(), vecMaximaSize[iGauss], vecMaximaSize[iGauss]/10.);
		params.Add(ostrSpread.str(), HWHM2SIGMA*vecMaximaWidth[iGauss], HWHM2SIGMA*vecMaximaWidth[iGauss]/10.);
		params.Add(ostrX0.str(), vecMaximaX[iGauss], vecMaximaX[iGauss]/10.);

		params.SetLimits(ostrAmp.str(), dMin, dMax);
		params.SetLowerLimit(ostrSpread.str(), 0.);
	}

	params.Add("offs", dMin, (dMax-dMin)/10.);
	params.SetLimits("offs", dMin, dMax);

	bool bValidFit=false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;

	for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
	{
		std::ostringstream ostrAmp, ostrSpread, ostrX0;
		ostrAmp << "amp_" << iGauss;
		ostrSpread << "spread_" << iGauss;
		ostrX0 << "x0_" << iGauss;

		// fix all other peaks
		for(unsigned int iGaussOther=0; iGaussOther<iNumGauss; ++iGaussOther)
		{
			if(iGaussOther == iGauss)
				continue;

			std::ostringstream ostrAmpO, ostrSpreadO, ostrX0O;
			ostrAmpO << "amp_" << iGaussOther;
			ostrSpreadO << "spread_" << iGaussOther;
			ostrX0O << "x0_" << iGaussOther;

			params.Fix(ostrAmpO.str());
			params.Fix(ostrSpreadO.str());
			params.Fix(ostrX0O.str());
		}

		params.Release(ostrAmp.str());
		params.Release(ostrSpread.str());
		params.Fix(ostrX0.str());

		params.Fix("offs");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue(ostrAmp.str(), mini.UserState().Value(ostrAmp.str()));
		params.SetError(ostrAmp.str(), mini.UserState().Error(ostrAmp.str()));

		params.SetValue(ostrSpread.str(), mini.UserState().Value(ostrSpread.str()));
		params.SetError(ostrSpread.str(), mini.UserState().Error(ostrSpread.str()));

		//params.SetValue(ostrX0.str(), mini.UserState().Value(ostrX0.str()));
		//params.SetError(ostrX0.str(), mini.UserState().Error(ostrX0.str()));

		//params.SetValue("offs", mini.UserState().Value("offs"));
		//params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}

	
	for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
	{
		std::ostringstream ostrAmp, ostrSpread, ostrX0;
		ostrAmp << "amp_" << iGauss;
		ostrSpread << "spread_" << iGauss;
		ostrX0 << "x0_" << iGauss;

		// fix all other peaks
		for(unsigned int iGaussOther=0; iGaussOther<iNumGauss; ++iGaussOther)
		{
			if(iGaussOther == iGauss)
				continue;

			std::ostringstream ostrAmpO, ostrSpreadO, ostrX0O;
			ostrAmpO << "amp_" << iGaussOther;
			ostrSpreadO << "spread_" << iGaussOther;
			ostrX0O << "x0_" << iGaussOther;

			params.Fix(ostrAmpO.str());
			params.Fix(ostrSpreadO.str());
			params.Fix(ostrX0O.str());
		}

		params.Fix(ostrAmp.str());
		params.Fix(ostrSpread.str());
		params.Release(ostrX0.str());

		params.Release("offs");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue(ostrX0.str(), mini.UserState().Value(ostrX0.str()));
		params.SetError(ostrX0.str(), mini.UserState().Error(ostrX0.str()));

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}


	// release all parameters
    //if(0)
	{
		for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
		{
			std::ostringstream ostrAmp, ostrSpread, ostrX0;
			ostrAmp << "amp_" << iGauss;
			ostrSpread << "spread_" << iGauss;
			ostrX0 << "x0_" << iGauss;

			params.Release(ostrAmp.str());
			params.Release(ostrSpread.str());
			params.Release(ostrX0.str());

			params.Release("offs");
		}

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
		{
			std::ostringstream ostrAmp, ostrSpread, ostrX0;
			ostrAmp << "amp_" << iGauss;
			ostrSpread << "spread_" << iGauss;
			ostrX0 << "x0_" << iGauss;

			params.SetValue(ostrAmp.str(), mini.UserState().Value(ostrAmp.str()));
			params.SetError(ostrAmp.str(), mini.UserState().Error(ostrAmp.str()));

			params.SetValue(ostrSpread.str(), mini.UserState().Value(ostrSpread.str()));
			params.SetError(ostrSpread.str(), mini.UserState().Error(ostrSpread.str()));

			params.SetValue(ostrX0.str(), mini.UserState().Value(ostrX0.str()));
			params.SetError(ostrX0.str(), mini.UserState().Error(ostrX0.str()));
		}

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}


	// release all limits
    //if(0)
	{
		for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
		{
			std::ostringstream ostrAmp, ostrSpread, ostrX0;
			ostrAmp << "amp_" << iGauss;
			ostrSpread << "spread_" << iGauss;
			ostrX0 << "x0_" << iGauss;

			params.RemoveLimits(ostrAmp.str());
			params.RemoveLimits(ostrSpread.str());
			params.RemoveLimits(ostrX0.str());
		}
		params.RemoveLimits("offs");


		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		for(unsigned int iGauss=0; iGauss<iNumGauss; ++iGauss)
		{
			std::ostringstream ostrAmp, ostrSpread, ostrX0;
			ostrAmp << "amp_" << iGauss;
			ostrSpread << "spread_" << iGauss;
			ostrX0 << "x0_" << iGauss;

			params.SetValue(ostrAmp.str(), mini.UserState().Value(ostrAmp.str()));
			params.SetError(ostrAmp.str(), mini.UserState().Error(ostrAmp.str()));

			params.SetValue(ostrSpread.str(), mini.UserState().Value(ostrSpread.str()));
			params.SetError(ostrSpread.str(), mini.UserState().Error(ostrSpread.str()));

			params.SetValue(ostrX0.str(), mini.UserState().Value(ostrX0.str()));
			params.SetError(ostrX0.str(), mini.UserState().Error(ostrX0.str()));
		}

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}

	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();


	std::vector<MultiGaussParams> vecMultiParams;
	vecMultiParams.resize(iNumGauss);

	for(unsigned int iPara=0; iPara<iNumGauss; ++iPara)
	{
		std::ostringstream ostrPara;
		ostrPara << iPara;
		std::string strPara = ostrPara.str();

		vecMultiParams[iPara].m_amp = lastmini.UserState().Value("amp_" + strPara);
		vecMultiParams[iPara].m_spread = lastmini.UserState().Value("spread_" + strPara);
		vecMultiParams[iPara].m_x0 = lastmini.UserState().Value("x0_" + strPara);

		vecMultiParams[iPara].m_amperr = lastmini.UserState().Error("amp_" + strPara);
		vecMultiParams[iPara].m_spreaderr = lastmini.UserState().Error("spread_" + strPara);
		vecMultiParams[iPara].m_x0err = lastmini.UserState().Error("x0_" + strPara);


		vecMultiParams[iPara].m_spread = fabs(vecMultiParams[iPara].m_spread);

		vecMultiParams[iPara].m_amperr = fabs(vecMultiParams[iPara].m_amperr);
		vecMultiParams[iPara].m_spreaderr = fabs(vecMultiParams[iPara].m_spreaderr);
		vecMultiParams[iPara].m_x0err = fabs(vecMultiParams[iPara].m_x0err);
	}

	double doffs =lastmini.UserState().Value("offs");
	double doffserr =lastmini.UserState().Error("offs");
	bool bNormalized = gmod.IsNormalized();


	if(iFitterVerbosity >= 3)
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			std::cerr << "result of multi-gauss fit step " << (++uiMini) << std::endl;
			std::cerr << "==========================" << std::endl;
			std::cerr << mini << std::endl;
		}

		std::cerr << "values max: " << dMax << ", min: " << dMin << ", nchan=" << iLen << std::endl;
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}


	*pmodel = new MultiGaussModel(gmod);
	(*pmodel)->m_vecParams = vecMultiParams;
	(*pmodel)->m_offs = doffs;
	(*pmodel)->m_offserr = doffserr;
	(*pmodel)->m_bNormalized = bNormalized;
	(*pmodel)->Normalize();

	return bValidFit;
}
