/*
 * A MIEZE signal fitter for the oscillation monitors using Minuit2
 *
 * Author: Tobias Weber
 * Date: April 2012
 */

#ifndef __MIEZE_SIN__
#define __MIEZE_SIN__

#include "../fitter.h"


// sin function model with fixed frequency
class MiezeSinModel : public FunctionModel
{
	protected:
		double m_dfreq, m_dfreqerr;
		double m_damp, m_damperr;
		double m_dphase, m_dphaseerr;
		double m_doffs, m_doffserr;

	public:
		MiezeSinModel(double dFreq=1., double dAmp=1., double dPhase=0., double dOffs=0.,
			double dFreqErr=0., double dAmpErr=0., double dPhaseErr=0., double dOffsErr=0.);
		virtual ~MiezeSinModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;
		virtual FunctionModel* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		double GetContrast() const;
		double GetContrastErr() const;
		
		double GetFreq() const { return m_dfreq; }
		double GetAmp() const { return m_damp; }
		double GetPhase() const { return m_dphase; }
		double GetOffs() const { return m_doffs; }
		
		double GetFreqErr() const { return m_dfreqerr; }
		double GetAmpErr() const { return m_damperr; }
		double GetPhaseErr() const { return m_dphaseerr; }
		double GetOffsErr() const { return m_doffserr; }

		const char* GetModelName() const { return "mieze_sine"; }
		virtual std::vector<std::string> GetParamNames() const;
		virtual std::vector<double> GetParamValues() const;
		virtual std::vector<double> GetParamErrors() const;
};



bool get_mieze_contrast(double& dFreq, double& dNumOsc, unsigned int iLen,
					const double* px, const double* py, const double *pdy,
					MiezeSinModel** pmodel);

#endif
