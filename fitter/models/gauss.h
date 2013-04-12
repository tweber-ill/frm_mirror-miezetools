/*
 * A gauss fitter using Minuit
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#ifndef __FITTER_GAUSS__
#define __FITTER_GAUSS__

#include "../fitter.h"


// gauss model
class GaussModel : public FunctionModel
{
	protected:
		double m_amp, m_spread, m_x0;
		double m_amperr, m_spreaderr, m_x0err;
		bool m_bNormalized;

	public:
		GaussModel(double damp=1., double dspread=1., double dx0=0.,
				   double damperr=0., double dspreaderr=0., double dx0err=0.,
				   bool bNormalized=0);
		virtual ~GaussModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;
		virtual FunctionModel* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		double GetMean() const;
		double GetSigma() const;
		double GetFWHM() const;
		double GetHWHM() const;
		double GetAmp() const;

		double GetMeanErr() const;
		double GetSigmaErr() const;
		double GetFWHMErr() const;
		double GetHWHMErr() const;
		double GetAmpErr() const;

		void Normalize();
		bool IsNormalized() const { return m_bNormalized; }
};

bool get_gauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					GaussModel **pmodel);

#endif
