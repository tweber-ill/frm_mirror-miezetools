/**
 * mieze-tool
 * data fitting
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 12-jul-2013
 * @license GPLv3
 */

#include "fit_data.h"

#include "../fitter/models/freefit.h"
#include "../fitter/models/msin.h"
#include "../fitter/models/gauss.h"

#include "tlibs/helper/misc.h"
#include "tlibs/math/math.h"
#include "tlibs/phys/mieze.h"
#include "tlibs/log/log.h"

#include "../main/settings.h"

#include "../helper/mieze.h"
#include "../helper/mfourier.h"
#include "../helper/misc.h"


bool FitData::fit(const Data1& dat, const FitDataParams& params, FunctionModel** pFkt)
{
	const std::vector<double> *pvecDatX, *pvecDatY, *pvecDatYErr;
	const_cast<Data1&>(dat).GetData(&pvecDatX, &pvecDatY, &pvecDatYErr);

	double *px = tl::vec_to_array<double>(*pvecDatX);
	double *py = tl::vec_to_array<double>(*pvecDatY);
	double *pyerr = tl::vec_to_array<double>(*pvecDatYErr);
	autodeleter<double> _a0(px, 1);
	autodeleter<double> _a1(py, 1);
	autodeleter<double> _a2(pyerr, 1);
	const unsigned int iLen = pvecDatX->size();


	// Minuit doesn't handle errors that are exactly 0
	if(params.bAssumeErrorIfZero)
	{
		double dMinY = *std::min_element(py, py+iLen);
		double dMaxY = *std::max_element(py, py+iLen);
		for(unsigned int iErr=0; iErr<iLen; ++iErr)
		{
			if(pyerr[iErr] < std::numeric_limits<double>::min())
				pyerr[iErr] = dMaxY * 0.001;
		}
	}

	*pFkt = 0;
	bool bOk = 0;
	if(params.iFkt == FIT_MIEZE_SINE) 				// MIEZE sine
	{
		double dNumOsc = Settings::Get<double>("mieze/num_osc");
		double dFreq = ::get_mieze_freq(px, dat.GetLength(), dNumOsc);

		MiezeSinModel *pModel = 0;

		if(iLen >= 2)
			bOk = ::get_mieze_contrast(dFreq, dNumOsc, iLen, px, py, pyerr, &pModel);

		//if(pModel)
		//	std::cout << "C = " << pModel->GetContrast() << " +- " << pModel->GetContrastErr()
		//				<< ", phase = " << pModel->GetPhase()/M_PI*180. << " +- " << pModel->GetPhaseErr()/M_PI*180.
		//				<< std::endl;
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
		tl::log_err("Unknown fit function selected.");
		return false;
	}

	/*if(*pFkt)
	{
		std::cout << "Fit " << (bOk ? "ok" : "failed" ) << ": "
					<< **pFkt << std::endl;
	}*/

	return bOk;
}

Data1 FitData::mieze_sum_foils(const std::vector<Data1>& vecFoils, const std::vector<double>* pvecFoilPhases)
{
	if(vecFoils.size() == 0)
	{
		tl::log_err("No foils in dataset.");
		return Data1();
	}

	const bool bShiftToMeanPhase = 0;
	const double dNumOsc = Settings::Get<double>("mieze/num_osc");
	const unsigned int iNumFoils = vecFoils.size();
	const unsigned int iNumTC = vecFoils[0].GetLength();
	MFourier fourier(iNumTC);

	double *pdyTotal = new double[iNumTC];
	double *pdyerrTotal = new double[iNumTC];
	autodeleter<double> _a1(pdyTotal, 1);
	autodeleter<double> _a2(pdyerrTotal, 1);
	for(unsigned int iTc=0; iTc<iNumTC; ++iTc)
		pdyTotal[iTc] = pdyerrTotal[iTc] = 0.;

	double *pdPhases = new double[iNumTC];
	double *pdPhaseErrs = new double[iNumTC];
	autodeleter<double> _a7(pdPhases, 1);
	autodeleter<double> _a7err(pdPhaseErrs, 1);

	double dMeanPhase = 0.;
	double dTotalCnts = 0.;
	for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
	{
		pdPhases[iFoil] = 0.;
		const Data1 *dat = &vecFoils[iFoil];

		if(pvecFoilPhases)
		{
			pdPhases[iFoil] = (*pvecFoilPhases)[iFoil];
		}
		else
		{
			FitDataParams params;
			params.iFkt = FIT_MIEZE_SINE;
			FunctionModel *pFkt = 0;
			bool bOk = FitData::fit(*dat, params, &pFkt);
			MiezeSinModel *pModel = (MiezeSinModel*) pFkt;

			if(bOk && pModel)
			{
				pdPhases[iFoil] = pModel->GetPhase();
				pdPhaseErrs[iFoil] = pModel->GetPhaseErr();
			}

			//std::cout << "fit: " << pModel->print(1) << std::endl;
			if(pModel) delete pModel;
		}

		double dCnts = dat->SumY();
		dTotalCnts += dCnts;

		if(bShiftToMeanPhase)
			dMeanPhase += pdPhases[iFoil] * dCnts;
	}

	/*
	std::cout << "Phases of foils: ";
	for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
		std::cout << pdPhases[iFoil] << ", ";
	std::cout << "\nDifferences: ";
	for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
		std::cout << (pdPhases[iFoil]-pdPhases[0]) << ", ";
	std::cout << "\nErrors: ";
	for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
		std::cout << pdPhaseErrs[iFoil] << ", ";
	std::cout << std::endl;
	*/

	if(bShiftToMeanPhase)
	{
		dMeanPhase /= dTotalCnts;
		dMeanPhase = fmod(dMeanPhase, 2.*M_PI);
	}

	double *pdxFoil = new double[iNumTC];
	double *pdyFoil = new double[iNumTC];
	double *pdyFoilCorr = new double[iNumTC];
	autodeleter<double> _a8(pdxFoil, 1);
	autodeleter<double> _a9(pdyFoil, 1);
	autodeleter<double> _a10(pdyFoilCorr, 1);

	for(unsigned int iFoil=0; iFoil<iNumFoils; ++iFoil)
	{
		const Data1 *dat = &vecFoils[iFoil];
		dat->ToArray<double>(pdxFoil, pdyFoil, 0, 0);

		fourier.phase_correction_0(pdyFoil, pdyFoilCorr, (pdPhases[iFoil]-dMeanPhase)/dNumOsc);

		for(unsigned int iTc=0; iTc<iNumTC; ++iTc)
			pdyTotal[iTc] += pdyFoilCorr[iTc];
	}
	//std::cout << "mean phase: " << dMeanPhase/M_PI*180. << std::endl;

	for(unsigned int iTc=0; iTc<iNumTC; ++iTc)
		pdyerrTotal[iTc] = sqrt(pdyTotal[iTc]);


	Data1 result(iNumTC, pdxFoil, pdyTotal, pdyerrTotal);
	return result;
}
