/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data.h"
#include <limits>

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



Data2::Data2(unsigned int iW, unsigned int iH, const double* pDat, const double *pErr)
			: m_dTotal(0.),
			  	  m_dMin(std::numeric_limits<double>::max()),
			  	  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH);
	this->SetVals(pDat, pErr);
}

void Data2::SetVals(const double* pDat, const double *pErr)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(unsigned int iY=0; iY<m_iHeight; ++iY)
		for(unsigned int iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = pDat ? pDat[iY*m_iWidth + iX] : 0.;
			double dErr = pErr ? pErr[iY*m_iWidth + iX] : 0.;

			this->SetVal(iX, iY, dVal);
			this->SetErr(iX, iY, dErr);

			m_dTotal += dVal;
		}
}




Data3::Data3(unsigned int iW, unsigned int iH, unsigned int iT, const double* pDat, const double *pErr)
			: m_dTotal(0.),
			  m_dMin(std::numeric_limits<double>::max()),
			  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH, iT);
	this->SetVals(pDat, pErr);
}

void Data3::SetVals(const double* pDat, const double *pErr)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(unsigned int iT=0; iT<m_iDepth; ++iT)
		for(unsigned int iY=0; iY<m_iHeight; ++iY)
			for(unsigned int iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = pDat ? pDat[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;
				double dErr = pErr ? pErr[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;

				this->SetVal(iX, iY, iT, dVal);
				this->SetErr(iX, iY, iT, dErr);

				m_dTotal += dVal;
			}
}

Data2 Data3::GetVal(unsigned int iT) const
{
	Data2 dat(m_iWidth, m_iHeight);

	for(unsigned int iY=0; iY<m_iHeight; ++iY)
		for(unsigned int iX=0; iX<m_iWidth; ++iX)
		{
			dat.SetVal(iX, iY, iT<m_iDepth ? this->GetVal(iX, iY, iT) : 0.);
			dat.SetErr(iX, iY, iT<m_iDepth ? this->GetErr(iX, iY, iT) : 0.);
		}

	return dat;
}
