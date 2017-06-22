/**
 * spin-echo exponential function
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date June 2017
 * @license GPLv3
 */

#ifndef __MIEZE_EXP__
#define __MIEZE_EXP__

#include "tlibs/fit/minuit.h"


class MiezeExpModel : public tl::FitterFuncModel<double>
{
	public:
		static const double s_dhbar;	// hbar in mueV*ps

	protected:
		double m_dP0, m_dP0Err;
		double m_dGamma, m_dGammaErr;

	public:
		MiezeExpModel(double dP0=1., double dGamma=500.,
			double dP0Err=0., double dGammaErr=0.);
		virtual ~MiezeExpModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;
		virtual tl::FitterFuncModel<double>* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		double GetP0() const { return m_dP0; }
		double GetGamma() const { return m_dGamma; }

		double GetP0Err() const { return m_dP0Err; }
		double GetGammaErr() const { return m_dGammaErr; }

		const char* GetModelName() const { return "mieze_exp"; }
		virtual std::vector<std::string> GetParamNames() const;
		virtual std::vector<double> GetParamValues() const;
		virtual std::vector<double> GetParamErrors() const;
};



bool get_mieze_gamma(unsigned int iLen,
	const double* px, const double* py, const double *pdy,
	MiezeExpModel** pmodel, bool bFixP0=0);

#endif
