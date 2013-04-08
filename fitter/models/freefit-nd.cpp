/*
 * A n-dim free function fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: July 2012
 */

#include <math.h>
#include <limits>
#include <algorithm>
#include <sstream>

#include <Minuit2/FCNBase.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnPrint.h>

#include "freefit-nd.h"
#include "freefit.h"
#include "../chi2.h"


static std::string get_param_name(unsigned int iNum)
{
	std::ostringstream ostr;
	ostr << "x";
	ostr << iNum;

	return ostr.str();
}

//----------------------------------------------------------------------
// freefit model

FreeFktModel_nd::FreeFktModel_nd() : m_uiDim(0)
{}

FreeFktModel_nd::FreeFktModel_nd(unsigned int uiDim, const char* pcExp)
{
	m_uiDim = uiDim;

	if(pcExp)
	{
		std::vector<Symbol> vecFreeParams = GetFreeParams();
		m_parser.SetFreeParams(vecFreeParams);
		
		std::string strExpression = pcExp;
		m_parser.ParseExpression(strExpression);
	}
}

FreeFktModel_nd::FreeFktModel_nd(const FreeFktModel_nd& model)
{
	this->operator=(model);
}

const FreeFktModel_nd& FreeFktModel_nd::operator=(const FreeFktModel_nd& model)
{
	this->m_parser = model.m_parser;
	this->m_uiDim = model.m_uiDim;

	return *this;
}

FreeFktModel_nd::~FreeFktModel_nd()
{}

std::vector<Symbol> FreeFktModel_nd::GetFreeParams(const double *px) const
{
	std::vector<Symbol> vecFreeParams;

	for(unsigned int i=0; i<m_uiDim; ++i)
	{
		Symbol sym;
		sym.strIdent = get_param_name(i);
		if(px)
			sym.dVal = px[i];
		else
			sym.dVal = 0.;

		//std::cout << "free param: " << sym.strIdent << std::endl;
		vecFreeParams.push_back(sym);
	}

	return vecFreeParams;
}

bool FreeFktModel_nd::SetParams(const std::vector<double>& vecParams)
{
	std::vector<Symbol>& syms = m_parser.GetSymbols();
	
	if(syms.size() != vecParams.size())
	{
		std::cerr << syms.size() << " symbols in table, but "
				  << vecParams.size() << " symbols supplied."
				  << std::endl;

		return false;
	}

	for(unsigned int i=0; i<vecParams.size(); ++i)
		syms[i].dVal = vecParams[i];

	return true;
}

double FreeFktModel_nd::operator()(const double* px) const
{
	double dVal = const_cast<Parser&>(m_parser).EvalTree(px);	// !!

	/*
	std::cout << "f(";
	for(unsigned int iX=0; iX<m_uiDim; ++iX)
		std::cout << px[iX] << ", ";
	std::cout << ") = ";
	std::cout << dVal << std::endl;
	*/
	
	return dVal;
}

FunctionModel_nd* FreeFktModel_nd::copy() const
{
	return new FreeFktModel_nd(*this);
}

std::string FreeFktModel_nd::print() const
{
	return m_parser.GetExpression(true, true);
}



bool get_freefit_nd(unsigned int uiDim, unsigned int iLen,
					const double** ppx, const double* py, const double* pdy,
					const char* pcExp, const char* pcLimits, const char* pcHints,
					std::vector<std::string>& vecFittedNames,
					std::vector<double>& vecFittedParams,
					std::vector<double>& vecFittedErrs,
					FreeFktModel_nd** pFinalModel)
{
	//std::cout << "dim = " << uiDim << ", len = " << iLen << std::endl;
	
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


	FreeFktModel_nd freemod(uiDim, pcExp);
	if(!freemod.IsOk())
	{
		std::cerr << "Error: N-dim free function model could not be created." << std::endl;
		return 0;
	}
	
	Chi2Function_nd fkt(&freemod, iLen, ppx, py, pdy);

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
			if(dMinLim >= dMaxLim)
				dMinLim = dMaxLim;

			params.SetLimits(sym.strIdent, dMinLim, dMaxLim);
		}
	}

	
	// step 1: free fit (limited)
	ROOT::Minuit2::MnMigrad migrad(fkt, params, /*MINUIT_STRATEGY*/1);
	ROOT::Minuit2::FunctionMinimum mini = migrad();
	bool bValidFit = mini.IsValid() && mini.HasValidParameters();
	//if(bValidFit)
	{
		for(const Symbol& sym : syms)
		{
			params.SetValue(sym.strIdent.c_str(), mini.UserState().Value(sym.strIdent.c_str()));
			params.SetError(sym.strIdent.c_str(), mini.UserState().Error(sym.strIdent.c_str()));
		}
	}



	// step 2: free fit (unlimited)
	for(const Symbol& sym : syms)
		params.RemoveLimits(sym.strIdent.c_str());

	ROOT::Minuit2::MnMigrad migrad2(fkt, params, /*MINUIT_STRATEGY*/2);
	ROOT::Minuit2::FunctionMinimum mini2 = migrad2();
	bValidFit = mini2.IsValid() && mini2.HasValidParameters();


	for(Symbol& sym : syms)
	{
		vecFittedNames.push_back(sym.strIdent);

		double dVal = mini2.UserState().Value(sym.strIdent.c_str());
		double dErr = mini2.UserState().Error(sym.strIdent.c_str());
		dErr = fabs(dErr);

		vecFittedParams.push_back(dVal);
		vecFittedErrs.push_back(dErr);

		// also write found values back into the symbol table
		sym.dVal = dVal;
	}

	*pFinalModel = new FreeFktModel_nd(freemod);

	/*
	{
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		std::cerr << "result of user-defined fit step 1" << std::endl;
		std::cerr << "=================================" << std::endl;
		std::cerr << mini << std::endl;
		std::cerr << "result of user-defined fit step 2" << std::endl;
		std::cerr << "=================================" << std::endl;
		std::cerr << mini2 << std::endl;
		std::cerr << "--------------------------------------------------------------------------------" << std::endl;
	}*/

	return bValidFit;
}
