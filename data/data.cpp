/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data.h"

Data1::Data1(unsigned int uiNum, const double* pValsX, const double* pValsY,
														  const double *pErrsY, const double *pErrsX)
{
	if(uiNum)
	{
		this->SetLength(uiNum);
		for(unsigned int i=0; i<uiNum; ++i)
		{
			this->SetX(i, pValsX[i]);
			this->SetY(i, pValsY[i]);

			this->SetXErr(i, pErrsX?pErrsX[i]:0.);
			this->SetYErr(i, pErrsY?pErrsY[i]:0.);
		}
	}
}
