/**
 * mieze-tool
 * data fitting
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 12-jul-2013
 * @license GPLv3
 */

#ifndef __FIT_DATA_H__
#define __FIT_DATA_H__

#include "data.h"
#include "tlibs/fit/minuit.h"

#include <vector>


#define FIT_MIEZE_SINE 					0
#define FIT_GAUSSIAN 					1
#define FIT_MULTI_GAUSSIAN 				2
#define FIT_MIEZE_EXP 					3
#define FIT_MIEZE_EXP_FIXEDP0			4
#define FIT_INVALID						-1

#define FIT_MIEZE_SINE_PIXELWISE 		0
#define FIT_MIEZE_SINE_PIXELWISE_FFT 	1

struct FitDataParams
{
	int iFkt;
	bool bAssumeErrorIfZero;
	int iNumPeaks;

	FitDataParams()
		: iFkt(FIT_INVALID), bAssumeErrorIfZero(1), iNumPeaks(1)
	{}
};

class FitData
{
public:
	static bool fit(const Data1& dat, const FitDataParams& params, tl::FitterFuncModel<double>** pFkt);
	static Data1 mieze_sum_foils(const std::vector<Data1>& vecFoils, const std::vector<double>* pvecFoilPhases=0);
};

#endif
