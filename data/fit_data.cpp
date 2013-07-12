/*
 * mieze-tool
 * data fitting
 * @author tweber
 * @date 12-jul-2013
 */

#include "fit_data.h"
#include "../fitter/models/freefit.h"
#include "../fitter/models/msin.h"
#include "../fitter/models/gauss.h"
#include "../helper/misc.h"
#include "../helper/mieze.hpp"
#include "../settings.h"

bool FitData::fit(const Data1& dat, const FitDataParams& params, FunctionModel** pFkt)
{
	const std::vector<double> *pvecDatX, *pvecDatY, *pvecDatYErr;
	const_cast<Data1&>(dat).GetData(&pvecDatX, &pvecDatY, &pvecDatYErr);

	double *px = vec_to_array<double>(*pvecDatX);
	double *py = vec_to_array<double>(*pvecDatY);
	double *pyerr = vec_to_array<double>(*pvecDatYErr);
	autodeleter<double> _a0(px, 1);
	autodeleter<double> _a1(py, 1);
	autodeleter<double> _a2(pyerr, 1);
	const unsigned int iLen = pvecDatX->size();


	if(params.bAssumeErrorIfZero)
	{
		double dMaxY = *std::max_element(py, py+iLen);
		for(unsigned int iErr=0; iErr<iLen; ++iErr)
		{
			if(pyerr[iErr] < std::numeric_limits<double>::min())
				pyerr[iErr] = dMaxY * 0.1;
		}
	}

	*pFkt = 0;
	bool bOk = 0;
	if(params.iFkt == FIT_MIEZE_SINE) 				// MIEZE sine
	{
		double dNumOsc = Settings::Get<double>("mieze/num_osc");
		double dFreq = get_mieze_freq(px, dat.GetLength(), dNumOsc);

		MiezeSinModel *pModel = 0;

		if(iLen >= 2)
			bOk = ::get_mieze_contrast(dFreq, dNumOsc, iLen, px, py, pyerr, &pModel);

		if(pModel)
			std::cout << "C = " << pModel->GetContrast() << " +- " << pModel->GetContrastErr()
						<< ", phase = " << pModel->GetPhase()/M_PI*180. << " +- " << pModel->GetPhaseErr()/M_PI*180.
						<< std::endl;
		*pFkt = pModel;
	}
	else if(params.iFkt == FIT_GAUSSIAN) 			// Gaussian
	{
		GaussModel *pModel = 0;
		bOk = ::get_gauss(iLen, px, py, pyerr, &pModel);
		*pFkt = pModel;
	}
	else if(params.iFkt == FIT_MULTI_GAUSSIAN)
	{
		MultiGaussModel *pModel = 0;
		int iNumPeaks = params.iNumPeaks;
		bOk = ::get_multigauss(iLen, px, py, pyerr, &pModel, iNumPeaks);
		*pFkt = pModel;
	}
	else
	{
		std::cerr << "Error: Unknown fit function selected."
				  << std::endl;
		return false;
	}

	if(*pFkt)
	{
		std::cout << "Fit " << (bOk ? "(ok)" : "(failed)" ) << ": "
					<< **pFkt << std::endl;
	}

	return bOk;
}

Data1 FitData::mieze_sum_foils(const std::vector<Data1>& vecFoils)
{
	// TODO

	return vecFoils[0];
}
