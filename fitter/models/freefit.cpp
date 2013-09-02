/*
 * A free function fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#include <cmath>
#include <limits>
#include <algorithm>
#include <sstream>

#include <Minuit2/FCNBase.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnPrint.h>

#include "freefit.h"
#include "../chi2.h"

//----------------------------------------------------------------------
// freefit model

FreeFktModel::FreeFktModel()
{}

FreeFktModel::FreeFktModel(const char* pcExp)
{
	if(pcExp)
	{
		std::string strExpression = pcExp;
		m_parser.ParseExpression(strExpression);

		for(const Symbol& sym : m_parser.GetSymbols())
		{
			m_vecParamNames.push_back(sym.strIdent);
			m_vecParamVals.push_back(sym.dVal);
			m_vecParamErrs.push_back(0.);
		}
	}
}

FreeFktModel::FreeFktModel(const FreeFktModel& model)
{
	this->operator=(model);
}

const FreeFktModel& FreeFktModel::operator=(const FreeFktModel& model)
{
	this->m_parser = model.m_parser;
	return *this;
}

FreeFktModel::FreeFktModel(const Parser& parser)
			: m_parser(parser)
{}

FreeFktModel::~FreeFktModel() {}

bool FreeFktModel::SetParams(const std::vector<double>& vecParams)
{
	std::vector<Symbol>& syms = m_parser.GetSymbols();
	
	if(syms.size() != vecParams.size())
	{
		if(iFitterVerbosity >= 1)
			std::cerr << syms.size() << " symbols in table, but "
							<< vecParams.size() << " symbols supplied."
							<< std::endl;

		return false;
	}

	for(unsigned int i=0; i<vecParams.size(); ++i)
		syms[i].dVal = vecParams[i];

	return true;
}

double FreeFktModel::operator()(double x) const
{
	return const_cast<Parser&>(m_parser).EvalTree(x);	// !!
}

FunctionModel* FreeFktModel::copy() const
{
	return new FreeFktModel(m_parser);
}

std::string FreeFktModel::print(bool bFillInSyms) const
{
	return m_parser.GetExpression(bFillInSyms, false);
}





void freefit_get_hint(const std::string& strIdent, double& dHint, double &dErr,
					 const std::vector<ParameterHints>& vecHints)
{
	for(const ParameterHints& hints : vecHints)
	{
		if(strIdent == hints.strSym)
		{
			dHint = hints.dVal;
			dErr = hints.dErr;

			return;
		}
	}
}

bool freefit_get_limits(const std::string& strIdent, double &dMinLim, double& dMaxLim,
						const std::vector<ParameterLimits>& vecLimits)
{
	for(const ParameterLimits& limits : vecLimits)
	{
		if(strIdent == limits.strSym)
		{
			dMinLim = limits.dLower;
			dMaxLim = limits.dUpper;

			return true;
		}
	}

	return false;
}



bool get_freefit(unsigned int iLen,
					const double* px, const double* py, const double* pdy,
					const char* pcExp, const char* pcLimits, const char* pcHints,
					std::vector<std::string>& vecFittedNames,
					std::vector<double>& vecFittedParams,
					std::vector<double>& vecFittedErrs,
					FreeFktModel** pFinalModel)
{
	std::vector<ParameterLimits> vecLimits;
	std::vector<ParameterHints> vecHints;

	if(pcLimits)
	{
		std::string strLimits = pcLimits;
		vecLimits = parse_parameter_limits(strLimits);
	}

	if(pcHints)
	{
		std::string strHints = pcHints;
		vecHints = parse_parameter_hints(strHints);
	}


	FreeFktModel freemod(pcExp);
	if(!freemod.IsOk())
	{
		if(iFitterVerbosity >= 1)
			std::cerr << "Error: Free function model could not be created." << std::endl;
		return 0;
	}
	Chi2Function fkt(&freemod, iLen, px, py, pdy);

	const double *pdMax = std::max_element(py,py+iLen),
				  dMin = *std::min_element(py,py+iLen);
	const double dXMax = *std::max_element(px,px+iLen),
				 dXMin = *std::min_element(px,px+iLen);

	double dMax = *pdMax;

	if(dMax==dMin)
	{
		if(iFitterVerbosity >= 2)
			std::cerr << "Warning: min == max!" << std::endl;
		//return 0;
	}


	bool bValidFit = false;
	std::vector<Symbol>& syms = freemod.GetSymbols();


	ROOT::Minuit2::MnUserParameters params;
	for(const Symbol& sym : syms)
	{
		double dHint = sym.dVal;
		double dErr = dHint*0.1;
		freefit_get_hint(sym.strIdent, dHint, dErr, vecHints);

		params.Add(sym.strIdent.c_str(), dHint, dErr);

		double dMinLim, dMaxLim;
		if(freefit_get_limits(sym.strIdent, dMinLim, dMaxLim, vecLimits))
		{
			if(dMinLim > dMaxLim)
			{
				double dTmpLim = dMinLim;
				dMinLim = dMaxLim;
				dMaxLim = dTmpLim;
			}

			if(dMinLim != dMaxLim)
				params.SetLimits(sym.strIdent, dMinLim, dMaxLim);
			//else
			//	params.Fix(sym.strIdent);
		}
	}


	std::vector<ROOT::Minuit2::FunctionMinimum> minis;
	minis.reserve(2);
	
	{
		// step 1: free fit (limited)
		
		ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/1);
		ROOT::Minuit2::FunctionMinimum mini = migrad();
		bValidFit = mini.IsValid() && mini.HasValidParameters();
		//if(bValidFit)
		{
			for(const Symbol& sym : syms)
			{
				params.SetValue(sym.strIdent.c_str(), mini.UserState().Value(sym.strIdent.c_str()));
				params.SetError(sym.strIdent.c_str(), mini.UserState().Error(sym.strIdent.c_str()));
			}
		}

		minis.push_back(mini);
	}


	{
		// step 2: free fit (unlimited)

		for(const Symbol& sym : syms)
			params.RemoveLimits(sym.strIdent.c_str());

		ROOT::Minuit2::MnMigrad migrad2(fkt, params, /*MINUIT_STRATEGY*/2);
		ROOT::Minuit2::FunctionMinimum mini = migrad2();
		bValidFit = mini.IsValid() && mini.HasValidParameters();

		minis.push_back(mini);
	}


	const ROOT::Minuit2::FunctionMinimum& lastmini = *minis.rbegin();


	std::vector<std::string> vecParamNames;
	for(Symbol& sym : syms)
	{
		vecFittedNames.push_back(sym.strIdent);


		double dVal = lastmini.UserState().Value(sym.strIdent.c_str());
		double dErr = lastmini.UserState().Error(sym.strIdent.c_str());
		dErr = fabs(dErr);

		vecParamNames.push_back(sym.strIdent);
		vecFittedParams.push_back(dVal);
		vecFittedErrs.push_back(dErr);


		// also write found values back into the symbol table
		sym.dVal = dVal;
	}
	

	*pFinalModel = new FreeFktModel(freemod);
	(*pFinalModel)->m_vecParamNames = vecParamNames;
	(*pFinalModel)->m_vecParamVals = vecFittedParams;
	(*pFinalModel)->m_vecParamErrs = vecFittedErrs;

	if(iFitterVerbosity >= 3)
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;

		unsigned int uiMini=0;
		for(const auto& mini : minis)
		{
			std::cerr << "result of user-defined fit step " << (++uiMini) << std::endl;
			std::cerr << "=================================" << std::endl;
			std::cerr << mini << std::endl;
		}

		std::cerr << "values max: " << dMax << ", min: " << dMin << ", nchan=" << iLen << std::endl;
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}

	return bValidFit;
}
