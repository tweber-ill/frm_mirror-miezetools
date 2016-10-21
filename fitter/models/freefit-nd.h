/*
 * A n-dim free function fitter using Minuit
 *
 * @author: Tobias Weber
 * @date: July 2012
 * @license GPLv3
 */

#ifndef __FREEFIT_ND__
#define __FREEFIT_ND__

#include "../fitter.h"
#include "../parser.h"


// fit to a user-entered n-dimensional function
class FreeFktModel_nd : public FunctionModel_nd
{
	protected:
		Parser m_parser;
		unsigned int m_uiDim;

		std::vector<Symbol> GetFreeParams(const double *px=0) const;

	public:
		FreeFktModel_nd();
		FreeFktModel_nd(const FreeFktModel_nd& model);
		const FreeFktModel_nd& operator=(const FreeFktModel_nd& model);

		FreeFktModel_nd(unsigned int uiDim, const char* pcExp=0);
		virtual ~FreeFktModel_nd();

		virtual unsigned int GetDim() const
		{ return m_uiDim; }

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(const double* x) const;

		virtual FunctionModel_nd* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		const Node& GetRootNode() const
		{ return m_parser.GetRootNode(); }

		const std::vector<Symbol>& GetSymbols() const
		{ return m_parser.GetSymbols(); }
		std::vector<Symbol>& GetSymbols()
		{ return m_parser.GetSymbols(); }

		bool IsOk() const
		{ return m_parser.IsOk(); }

		const char* GetModelName() const { return "user_defined_ndim"; }
};

bool get_freefit_nd(unsigned int uiDim, unsigned int iLen,
					const double** ppx, const double* py, const double* pdy,
					const char* pcExp, const char* pcLimits, const char* pcHints,
					std::vector<std::string>& vecFittedNames,
					std::vector<double>& vecFittedParams,
					std::vector<double>& vecFittedErrs,
					FreeFktModel_nd** pFinalModel);

#endif
