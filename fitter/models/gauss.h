/**
 * A gauss fitter using Minuit
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date April 2012
 * @license GPLv3
 */

#ifndef __FITTER_GAUSS__
#define __FITTER_GAUSS__

#include "tlibs/fit/minuit.h"


// gauss model
class GaussModel : public tl::FitterFuncModel<double>
{
	protected:
		double m_amp, m_spread, m_x0;
		double m_amperr, m_spreaderr, m_x0err;
		double m_offs, m_offserr;
		bool m_bNormalized;

	public:
		GaussModel(double damp=1., double dspread=1., double dx0=0.,
				   double damperr=0., double dspreaderr=0., double dx0err=0.,
				   bool bNormalized=0,
				   double doffs=0., double doffserr=0.);
		virtual ~GaussModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;
		virtual tl::FitterFuncModel<double>* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		double GetMean() const;
		double GetSigma() const;
		double GetFWHM() const;
		double GetHWHM() const;
		double GetAmp() const;
		double GetOffs() const;

		double GetMeanErr() const;
		double GetSigmaErr() const;
		double GetFWHMErr() const;
		double GetHWHMErr() const;
		double GetAmpErr() const;
		double GetOffsErr() const;

		void Normalize();
		bool IsNormalized() const { return m_bNormalized; }

		const char* GetModelName() const { return "gaussian"; }
		virtual std::vector<std::string> GetParamNames() const;
		virtual std::vector<double> GetParamValues() const;
		virtual std::vector<double> GetParamErrors() const;
};


struct MultiGaussParams
{
	double m_amp, m_spread, m_x0;
	double m_amperr, m_spreaderr, m_x0err;

	MultiGaussParams()
	{
		m_amp = m_spread = m_x0 = 0.;
		m_amperr = m_spreaderr = m_x0err = 0.;
	}
};

// multi gauss model
class MultiGaussModel : public tl::FitterFuncModel<double>
{
	protected:
		bool m_bNormalized;
		std::vector<MultiGaussParams> m_vecParams;
		double m_offs, m_offserr;

	public:
		MultiGaussModel(unsigned int iNumGausses);
		MultiGaussModel(const MultiGaussModel& gauss);
		const MultiGaussModel& operator=(const MultiGaussModel& gauss);
		virtual ~MultiGaussModel();

		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(double x) const;
		virtual tl::FitterFuncModel<double>* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		double GetMean(unsigned int iNum) const;
		double GetSigma(unsigned int iNum) const;
		double GetFWHM(unsigned int iNum) const;
		double GetHWHM(unsigned int iNum) const;
		double GetAmp(unsigned int iNum) const;
		double GetOffs() const;

		double GetMeanErr(unsigned int iNum) const;
		double GetSigmaErr(unsigned int iNum) const;
		double GetFWHMErr(unsigned int iNum) const;
		double GetHWHMErr(unsigned int iNum) const;
		double GetAmpErr(unsigned int iNum) const;
		double GetOffsErr() const;

		unsigned int GetNumGausses() const { return m_vecParams.size(); }

		void Normalize();
		bool IsNormalized() const { return m_bNormalized; }

		const char* GetModelName() const { return "multi_gaussian"; }
		virtual std::vector<std::string> GetParamNames() const;
		virtual std::vector<double> GetParamValues() const;
		virtual std::vector<double> GetParamErrors() const;

		friend bool get_multigauss(unsigned int iLen,
				const double *px, const double *py, const double *pdy,
				MultiGaussModel **pmodel, unsigned int iNumGauss);
};


extern bool get_gauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					GaussModel **pmodel);

extern bool get_multigauss(unsigned int iLen,
					const double *px, const double *py, const double *pdy,
					MultiGaussModel **pmodel, unsigned int iNumGauss=2);

#endif
