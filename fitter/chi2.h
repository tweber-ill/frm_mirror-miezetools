/**
 * Chi^2 calculation
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date April 2012
 * @license GPLv3
 */

#ifndef __CHI_2__
#define __CHI_2__

#include <Minuit2/FCNBase.h>
#include <vector>

#include "fitter.h"

// generic chi^2 calculation
class Chi2Function : public ROOT::Minuit2::FCNBase
{
	protected:
		const FunctionModel *m_pfkt;

		unsigned int m_uiLen;
		const double* m_px;
		const double* m_py;
		const double* m_pdy;

		double m_dSigma = 1.;

	public:
		Chi2Function(const FunctionModel* fkt=0,
					   unsigned int uiLen=0, const double* px=0,
					   const double *py=0, const double *pdy=0)
					: m_pfkt(fkt), m_uiLen(uiLen), m_px(px), m_py(py), m_pdy(pdy)
		{}

		virtual ~Chi2Function()
		{}

		double chi2(const std::vector<double>& vecParams) const;

		virtual double Up() const
		{
			return m_dSigma*m_dSigma;
		}

		virtual double operator()(const std::vector<double>& vecParams) const
		{
			return chi2(vecParams);
		}

		void SetSigma(double dSig) { m_dSigma = dSig; }
		double GetSigma() const { return m_dSigma; }
};

// in n dimensions
class Chi2Function_nd : public ROOT::Minuit2::FCNBase
{
	protected:
		const FunctionModel_nd *m_pfkt;
		unsigned int m_uiDim;

		unsigned int m_uiLen;
		std::vector<const double*> m_vecpx;

		const double* m_py;
		const double* m_pdy;

	public:
		Chi2Function_nd(const FunctionModel_nd* fkt=0,
					   unsigned int uiLen=0, const double** ppx=0,
					   const double *py=0, const double *pdy=0)
					: m_pfkt(fkt), m_uiDim(fkt->GetDim()), m_uiLen(uiLen), 
					  m_py(py), m_pdy(pdy)
		{
			m_vecpx.resize(m_uiDim);

			for(unsigned int i=0; i<m_uiDim; ++i)
				m_vecpx[i] = ppx[i];
		}

		virtual ~Chi2Function_nd()
		{}

		double chi2(const std::vector<double>& vecParams) const;

		virtual double Up() const
		{
			// 1. for chi^2
			return 1.;
		}

		virtual double operator()(const std::vector<double>& vecParams) const
		{
			return chi2(vecParams);
		}
};

#endif
