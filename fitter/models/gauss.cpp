/*
 * A gauss fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: April 2012, 25-apr-2013
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
#include "../../helper/misc.h"

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
			 << m_spread << ")**2) + " << m_offs;
	}
	else
	{
		ostr << "amp";
		if(m_bNormalized)
			ostr << "/sqrt(2*pi * sigma)";
		ostr << " * exp(-0.5 * ((x-x0) / sigma)**2) + offs";
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

	double dAmpErr = lastmini.UserState().Error("amp");
	double dSpreadErr = lastmini.UserState().Error("spread");
	double dx0Err = lastmini.UserState().Error("x0");
	double doffserr =lastmini.UserState().Error("offs");


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
				 << m_vecParams[i].m_spread << ")**2) + ";
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
			ostr << " * exp(-0.5 * ((x-x0_" <<i<<") / sigma_"<<i<<")**2) + ";
		}

		ostr << "offs";
	}
	return ostr.str();
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


bool get_doublegauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					MultiGaussModel **pmodel)
{
	MultiGaussModel gmod(2);
	Chi2Function fkt(&gmod, iLen, px, py, pdy);

	typedef std::pair<const double*, const double*> t_minmax;
	t_minmax minmax_x = boost::minmax_element(px, px+iLen);
	t_minmax minmax_y = boost::minmax_element(py, py+iLen);

	const double dXMin = *minmax_x.first;
	const double dXMax = *minmax_x.second;

	const double *pdMax = minmax_y.second;
	const double dMin = *minmax_y.first;
	double dMax = *pdMax;

	if(dMax==dMin)
	{
		std::cerr << "Error: min == max, won't try fitting!" << std::endl;
		return 0;
	}

	ROOT::Minuit2::MnUserParameters params;
	params.Add("amp_0", 400.-105., 50.);
	params.Add("spread_0", FWHM2SIGMA*0.07, FWHM2SIGMA*0.01);
	params.Add("x0_0", 0.01, 0.001);

	params.Add("amp_1", 275.-105., 50.);
	params.Add("spread_1", FWHM2SIGMA*0.06, FWHM2SIGMA*0.01);
	params.Add("x0_1", 0.139, 0.005);

	params.Add("offs", 105., 10.);


	params.SetLimits("offs", dMin, dMax);
	params.SetLimits("amp_0", dMin, dMax);
	params.SetLimits("amp_1", dMin, dMax);
	params.SetLowerLimit("spread_0", 0.);
	params.SetLowerLimit("spread_1", 0.);


	bool bValidFit=false;
	std::vector<ROOT::Minuit2::FunctionMinimum> minis;

	{
		params.Fix("amp_1");
		params.Fix("spread_1");
		params.Fix("x0_1");
		//params.Fix("offs");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue("amp_0", mini.UserState().Value("amp_0"));
		params.SetError("amp_0", mini.UserState().Error("amp_0"));

		params.SetValue("spread_0", mini.UserState().Value("spread_0"));
		params.SetError("spread_0", mini.UserState().Error("spread_0"));

		params.SetValue("x0_0", mini.UserState().Value("x0_0"));
		params.SetError("x0_0", mini.UserState().Error("x0_0"));

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}


	{
		params.Fix("amp_0");
		params.Fix("spread_0");
		params.Fix("x0_0");
		//params.Fix("offs");

		params.Release("amp_1");
		params.Release("spread_1");
		params.Release("x0_1");

		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue("amp_1", mini.UserState().Value("amp_1"));
		params.SetError("amp_1", mini.UserState().Error("amp_1"));

		params.SetValue("spread_1", mini.UserState().Value("spread_1"));
		params.SetError("spread_1", mini.UserState().Error("spread_1"));

		params.SetValue("x0_1", mini.UserState().Value("x0_1"));
		params.SetError("x0_1", mini.UserState().Error("x0_1"));

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}


	{
		params.Release("amp_0");
		params.Release("spread_0");
		params.Release("x0_0");

		params.Release("amp_1");
		params.Release("spread_1");
		params.Release("x0_1");

		params.Release("offs");


		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue("amp_0", mini.UserState().Value("amp_0"));
		params.SetError("amp_0", mini.UserState().Error("amp_0"));

		params.SetValue("spread_0", mini.UserState().Value("spread_0"));
		params.SetError("spread_0", mini.UserState().Error("spread_0"));

		params.SetValue("x0_0", mini.UserState().Value("x0_0"));
		params.SetError("x0_0", mini.UserState().Error("x0_0"));

		params.SetValue("amp_1", mini.UserState().Value("amp_1"));
		params.SetError("amp_1", mini.UserState().Error("amp_1"));

		params.SetValue("spread_1", mini.UserState().Value("spread_1"));
		params.SetError("spread_1", mini.UserState().Error("spread_1"));

		params.SetValue("x0_1", mini.UserState().Value("x0_1"));
		params.SetError("x0_1", mini.UserState().Error("x0_1"));

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}


	{
		params.RemoveLimits("amp_0");
		params.RemoveLimits("spread_0");
		params.RemoveLimits("x0_0");

		params.RemoveLimits("amp_1");
		params.RemoveLimits("spread_1");
		params.RemoveLimits("x0_1");

		params.RemoveLimits("offs");


		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		params.SetValue("amp_0", mini.UserState().Value("amp_0"));
		params.SetError("amp_0", mini.UserState().Error("amp_0"));

		params.SetValue("spread_0", mini.UserState().Value("spread_0"));
		params.SetError("spread_0", mini.UserState().Error("spread_0"));

		params.SetValue("x0_0", mini.UserState().Value("x0_0"));
		params.SetError("x0_0", mini.UserState().Error("x0_0"));

		params.SetValue("amp_1", mini.UserState().Value("amp_1"));
		params.SetError("amp_1", mini.UserState().Error("amp_1"));

		params.SetValue("spread_1", mini.UserState().Value("spread_1"));
		params.SetError("spread_1", mini.UserState().Error("spread_1"));

		params.SetValue("x0_1", mini.UserState().Value("x0_1"));
		params.SetError("x0_1", mini.UserState().Error("x0_1"));

		params.SetValue("offs", mini.UserState().Value("offs"));
		params.SetError("offs", mini.UserState().Error("offs"));

		minis.push_back(mini);
	}

	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();

	test:


	std::vector<MultiGaussParams> vecMultiParams;
	vecMultiParams.resize(2);

	for(unsigned int iPara=0; iPara<vecMultiParams.size(); ++iPara)
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


	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			std::cerr << "result of double-gauss fit step " << (++uiMini) << std::endl;
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
