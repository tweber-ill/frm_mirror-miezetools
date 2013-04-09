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

	if(pDat)
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

	if(pDat)
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

	double dTotal = 0.;
	for(unsigned int iY=0; iY<m_iHeight; ++iY)
		for(unsigned int iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = iT<m_iDepth ? this->GetVal(iX, iY, iT) : 0.;

			dat.SetVal(iX, iY, dVal);
			dat.SetErr(iX, iY, iT<m_iDepth ? this->GetErr(iX, iY, iT) : 0.);

			dTotal += dVal;
		}

	dat.SetTotal(dTotal);
	return dat;
}

Data1 Data3::GetXYSum() const
{
	Data1 dat;
	dat.SetLength(this->GetDepth());

	for(uint iT=0; iT<GetDepth(); ++iT)
	{
		double dSum = 0.;
		double dErrSum = 0.;

		for(uint iY=0; iY<GetHeight(); ++iY)
			for(uint iX=0; iX<GetWidth(); ++iX)
			{
				dSum += GetVal(iX, iY, iT);
				dErrSum += GetErr(iX, iY, iT);
			}

		dat.SetX(iT, iT);
		dat.SetXErr(iT, 0.);
		dat.SetY(iT, dSum);
		dat.SetYErr(iT, dErrSum);
	}

	return dat;
}



Data4::Data4(unsigned int iW, unsigned int iH, unsigned int iD, unsigned int iD2, const double* pDat, const double *pErr)
			: m_dTotal(0.),
			  m_dMin(std::numeric_limits<double>::max()),
			  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH, iD, iD2);

	if(pDat)
		this->SetVals(pDat, pErr);
}

void Data4::SetVal(unsigned int iX, unsigned int iY, unsigned int iD, unsigned int iD2, double dVal)
{
	m_vecVals[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal;
	m_dMin = std::min(m_dMin, dVal);
	m_dMax = std::max(m_dMax, dVal);
}

void Data4::SetVals(const double* pDat, const double *pErr)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(unsigned int iD2=0; iD2<m_iDepth2; ++iD2)
		SetVals(iD2, pDat+iD2*m_iDepth*m_iWidth*m_iHeight, pErr+iD2*m_iDepth*m_iWidth*m_iHeight);
}

void Data4::SetVals(unsigned int iD2, const double *pDat, const double *pErr)
{
	for(unsigned int iD=0; iD<m_iDepth; ++iD)
		for(unsigned int iY=0; iY<m_iHeight; ++iY)
			for(unsigned int iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = pDat ? pDat[iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;
				double dErr = pErr ? pErr[iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;

				this->SetVal(iX, iY, iD, iD2, dVal);
				this->SetErr(iX, iY, iD, iD2, dErr);

				m_dTotal += dVal;
			}
}

Data2 Data4::GetVal(unsigned int iD, unsigned int iD2) const
{
	Data2 dat(m_iWidth, m_iHeight);

	double dTotal = 0.;
	for(unsigned int iY=0; iY<m_iHeight; ++iY)
		for(unsigned int iX=0; iX<m_iWidth; ++iX)
		{
			double dVal=0., dErr=0.;
			if(iD<m_iDepth && iD2<m_iDepth2)
			{
				dVal = this->GetVal(iX, iY, iD, iD2);
				dErr = this->GetErr(iX, iY, iD, iD2);
			}

			dat.SetVal(iX, iY, dVal);
			dat.SetErr(iX, iY, dErr);

			dTotal += dVal;
		}

	dat.SetTotal(dTotal);
	return dat;
}

Data1 Data4::GetXYSum(unsigned int iD2) const
{
	Data1 dat;
	dat.SetLength(this->GetDepth());

	for(uint iT=0; iT<GetDepth(); ++iT)
	{
		double dSum = 0.;
		double dErrSum = 0.;

		for(uint iY=0; iY<GetHeight(); ++iY)
			for(uint iX=0; iX<GetWidth(); ++iX)
			{
				dSum += GetVal(iX, iY, iT, iD2);
				dErrSum += GetErr(iX, iY, iT, iD2);
			}

		dat.SetX(iT, iT);
		dat.SetXErr(iT, 0.);
		dat.SetY(iT, dSum);
		dat.SetYErr(iT, dErrSum);
	}

	return dat;
}
