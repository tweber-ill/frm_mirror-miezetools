/**
 * A gauss n-dim fitter using Minuit
 *
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date July 2012
 * @license GPLv3
 */

#ifndef __GAUSS_ND_H__
#define __GAUSS_ND_H__

#include "../fitter.h"


// gauss nd model
class GaussModel_nd : public FunctionModel_nd
{
	protected:
		unsigned int m_uiDim;
		double m_amp, *m_pspread, *m_px0;

	public:
		GaussModel_nd(unsigned int uiDim,
					  double amp=0., const double* pspread=0, const double* px0=0);
		virtual ~GaussModel_nd();

		virtual unsigned int GetDim() const;
		virtual bool SetParams(const std::vector<double>& vecParams);
		virtual double operator()(const double* x) const;
		virtual FunctionModel_nd* copy() const;
		virtual std::string print(bool bFillInSyms=true) const;

		const char* GetModelName() const { return "gaussian_ndim"; }
};

int get_gauss_nd(unsigned int uiDim, unsigned int iLen,
				const double **ppx, const double *py, const double *pdy,
				double& dAmp, double *pSpread, double *pX0,
				double& dAmp_err, double *pSpread_err, double *pX0_err);

#endif
