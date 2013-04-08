/*
 * A free function fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#ifndef __FREEFIT__
#define __FREEFIT__

#include "../fitter.h"
#include "../parser.h"


// fit to a user-entered function
class FreeFktModel : public FunctionModel
{
	protected:
		Parser m_parser;

	public:
		FreeFktModel();
		FreeFktModel(const FreeFktModel& model);
		const FreeFktModel& operator=(const FreeFktModel& model);

		FreeFktModel(const char* pcExp);
		FreeFktModel(const Parser& parser);
		virtual ~FreeFktModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;

		virtual FunctionModel* copy() const;
		virtual std::string print() const;

		const Node& GetRootNode() const
		{ return m_parser.GetRootNode(); }
		const std::vector<Symbol>& GetSymbols() const
		{ return m_parser.GetSymbols(); }
		std::vector<Symbol>& GetSymbols()
		{ return m_parser.GetSymbols(); }

		bool IsOk() const
		{ return m_parser.IsOk(); }
};

void freefit_get_hint(const std::string& strIdent, double& dHint, double &dErr,
					 const std::vector<ParameterHints>& vecHints);
bool freefit_get_limits(const std::string& strIdent, double &dMinLim, double& dMaxLim,
						const std::vector<ParameterLimits>& vecLimits);

bool get_freefit(unsigned int iLen,
					const double* px, const double* py, const double* pdy,
					const char* pcExp, const char* pcLimits, const char* pcHints,
					std::vector<std::string>& vecFittedNames,
					std::vector<double>& vecFittedParams,
					std::vector<double>& vecFittedErrs,
					FreeFktModel** pFinalModel);

#endif
