/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data.h"
#include <limits>
#include <boost/algorithm/minmax_element.hpp>

Data1::Data1(uint uiNum, const double* pValsX, const double* pValsY,
														  const double *pErrsY, const double *pErrsX)
{
	if(uiNum)
	{
		this->SetLength(uiNum);
		for(uint i=0; i<uiNum; ++i)
		{
			this->SetX(i, pValsX[i]);
			this->SetY(i, pValsY[i]);

			this->SetXErr(i, pErrsX?pErrsX[i]:0.);
			this->SetYErr(i, pErrsY?pErrsY[i]:0.);
		}
	}
}

void Data1::clear()
{
	m_vecValsX.clear();
	m_vecValsY.clear();
	m_vecErrsX.clear();
	m_vecErrsY.clear();
}

void Data1::SetLength(uint uiLen)
{
	m_vecValsX.resize(uiLen);
	m_vecValsY.resize(uiLen);

	m_vecErrsX.resize(uiLen);
	m_vecErrsY.resize(uiLen);
}

double Data1::SumY() const
{
	double dTotal = 0.;
	for(uint i=0; i<GetLength(); ++i)
		dTotal += GetY(i);
	return dTotal;
}

void Data1::GetData(const std::vector<double> **pvecX, const std::vector<double> **pvecY,
			const std::vector<double> **pvecYErr, const std::vector<double> **pvecXErr)
{
	if(IsRoiActive())
	{
		m_vecValsXRoiTmp.clear();
		m_vecValsYRoiTmp.clear();
		m_vecErrsXRoiTmp.clear();
		m_vecErrsYRoiTmp.clear();

		for(uint iVal=0; iVal<m_vecValsX.size(); ++iVal)
		{
			double dX = m_vecValsX[iVal];
			double dY = m_vecValsY[iVal];

			if(IsInsideRoi(dX, dY))
			{
				m_vecValsXRoiTmp.push_back(dX);
				m_vecValsYRoiTmp.push_back(dY);

				m_vecErrsXRoiTmp.push_back(m_vecErrsX[iVal]);
				m_vecErrsYRoiTmp.push_back(m_vecErrsY[iVal]);
			}
		}

		if(pvecX) *pvecX = &m_vecValsXRoiTmp;
		if(pvecY) *pvecY = &m_vecValsYRoiTmp;
		if(pvecXErr) *pvecXErr = &m_vecErrsXRoiTmp;
		if(pvecYErr) *pvecYErr = &m_vecErrsYRoiTmp;
	}
	else
	{
		if(pvecX) *pvecX = &m_vecValsX;
		if(pvecY) *pvecY = &m_vecValsY;
		if(pvecXErr) *pvecXErr = &m_vecErrsX;
		if(pvecYErr) *pvecYErr = &m_vecErrsY;
	}
}

void Data1::GetXMinMax(double& dXMin, double& dXMax) const
{
	typedef std::vector<double>::const_iterator t_iter;
	std::pair<t_iter, t_iter> pair =
			boost::minmax_element(m_vecValsX.begin(), m_vecValsX.end());

	dXMin = (pair.first==m_vecValsX.end() ? 0. : *pair.first);
	dXMax = (pair.second==m_vecValsX.end() ? 0. : *pair.second);
}

void Data1::GetYMinMax(double& dYMin, double& dYMax) const
{
	typedef std::vector<double>::const_iterator t_iter;
	std::pair<t_iter, t_iter> pair =
			boost::minmax_element(m_vecValsY.begin(), m_vecValsY.end());

	dYMin = (pair.first==m_vecValsY.end()? 0.: *pair.first);
	dYMax = (pair.second==m_vecValsY.end()? 0.: *pair.second);
}

//------------------------------------------------------------------------


Data2::Data2(uint iW, uint iH, const double* pDat, const double *pErr)
			: m_dTotal(0.),
			  	  m_dMin(std::numeric_limits<double>::max()),
			  	  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH);

	if(pDat)
		this->SetVals(pDat, pErr);

	SetXRange(0., m_iWidth-1);
	SetYRange(0., m_iHeight-1);
}

void Data2::SetZero()
{
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				SetVal(iX, iY, 0.);
				SetErr(iX, iY, 0.);
			}

	m_dMin = m_dMax = m_dTotal = 0.;
}

void Data2::SetSize(uint iWidth, uint iHeight)
{
	if(m_iWidth==iWidth && m_iHeight==iHeight)
		return;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_vecVals.resize(m_iWidth * m_iHeight);
	m_vecErrs.resize(m_iWidth * m_iHeight);
}

double Data2::GetValRaw(uint iX, uint iY) const
{
	return m_vecVals[iY*m_iWidth + iX];
}

double Data2::GetErrRaw(uint iX, uint iY) const
{
	return m_vecErrs[iY*m_iWidth + iX];
}

double Data2::GetVal(uint iX, uint iY) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetValRaw(iX, iY);
	return 0.;
}
double Data2::GetErr(uint iX, uint iY) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetErrRaw(iX, iY);
	return 0.;
}

void Data2::SetVal(uint iX, uint iY, double dVal)
{
	m_vecVals[iY*m_iWidth + iX] = dVal;
	m_dMin = std::min(m_dMin, dVal);
	m_dMax = std::max(m_dMax, dVal);
}

void Data2::SetErr(uint iX, uint iY, double dVal)
{
	m_vecErrs[iY*m_iWidth + iX] = dVal;
}

void Data2::SetVals(const double* pDat, const double *pErr)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = pDat ? pDat[iY*m_iWidth + iX] : 0.;
			double dErr = pErr ? pErr[iY*m_iWidth + iX] : 0.;

			this->SetVal(iX, iY, dVal);
			this->SetErr(iX, iY, dErr);

			m_dTotal += dVal;
		}
}

Data1 Data2::SumY() const
{
	Data1 dat1;
	dat1.SetLength(GetWidth());

	for(uint iX=0; iX<GetWidth(); ++iX)
	{
		double dVal = 0.;
		for(uint iY=0; iY<GetHeight(); ++iY)
			dVal += this->GetVal(iX, iY);

		dat1.SetX(iX, HasRange()?GetRangeXPos(iX):double(iX));
		dat1.SetXErr(iX, 0.);

		dat1.SetY(iX, dVal);
		dat1.SetYErr(iX, sqrt(dVal));
	}

	return dat1;
}


void Data2::FromMatrix(const ublas::matrix<double>& mat)
{
	this->SetSize(mat.size1(), mat.size2());

	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = mat(iX, iY);
			this->SetVal(iX, iY, dVal);
			this->SetErr(iX, iY, 0.);

			m_dTotal += dVal;
		}
}


//------------------------------------------------------------------------




Data3::Data3(uint iW, uint iH, uint iT, const double* pDat, const double *pErr)
			: m_iDepth(0),
			  m_dTotal(0.),
			  m_dMin(std::numeric_limits<double>::max()),
			  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH, iT);

	if(pDat)
		this->SetVals(pDat, pErr);

	SetXRange(0., m_iWidth-1);
	SetYRange(0., m_iHeight-1);
}

void Data3::SetZero()
{
	for(uint iT=0; iT<m_iDepth; ++iT)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				SetVal(iX, iY, iT, 0.);
				SetErr(iX, iY, iT, 0.);
			}

	m_dMin = m_dMax = m_dTotal = 0.;
}

void Data3::Add(const Data3& dat)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iT=0; iT<m_iDepth; ++iT)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				const double dNewVal = GetVal(iX, iY, iT)+dat.GetVal(iX, iY, iT);
				const double dNewErr = GetErr(iX, iY, iT)+dat.GetErr(iX, iY, iT);

				SetVal(iX, iY, iT, dNewVal);
				SetErr(iX, iY, iT, dNewErr);

				m_dTotal += dNewVal;
			}
}

void Data3::SetSize(uint iWidth, uint iHeight, uint iDepth)
{
	if(m_iWidth==iWidth && m_iHeight==iHeight && m_iDepth==iDepth)
		return;

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;

	m_vecVals.resize(m_iWidth * m_iHeight * m_iDepth);
	m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth);
}

double Data3::GetValRaw(uint iX, uint iY, uint iT) const
{
	return m_vecVals[iT*m_iWidth*m_iHeight +
	                 	 	 iY*m_iWidth + iX];
}
double Data3::GetErrRaw(uint iX, uint iY, uint iT) const
{
	return m_vecErrs[iT*m_iWidth*m_iHeight +
	                 	 	 iY*m_iWidth + iX];
}

double Data3::GetVal(uint iX, uint iY, uint iT) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetValRaw(iX, iY, iT);
	return 0.;
}
double Data3::GetErr(uint iX, uint iY, uint iT) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetErrRaw(iX, iY, iT);
	return 0.;
}

void Data3::SetVal(uint iX, uint iY, uint iT, double dVal)
{
	m_vecVals[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal;
	m_dMin = std::min(m_dMin, dVal);
	m_dMax = std::max(m_dMax, dVal);
}
void Data3::SetErr(uint iX, uint iY, uint iT, double dVal)
{
	m_vecErrs[iT*m_iWidth*m_iHeight +
	          	  	  iY*m_iWidth + iX] = dVal;
}

void Data3::SetVals(const double* pDat, const double *pErr)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iT=0; iT<m_iDepth; ++iT)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = pDat ? pDat[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;
				double dErr = pErr ? pErr[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;

				this->SetVal(iX, iY, iT, dVal);
				this->SetErr(iX, iY, iT, dErr);

				m_dTotal += dVal;
			}
}

Data2 Data3::GetVal(uint iT) const
{
	Data2 dat(m_iWidth, m_iHeight);
	dat.CopyXYRangeFrom(this);
	dat.CopyRoiFlagsFrom(this);

	double dTotal = 0.;
	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = iT<m_iDepth ? this->GetVal(iX, iY, iT) : 0.;

			dat.SetVal(iX, iY, dVal);
			dat.SetErr(iX, iY, iT<m_iDepth ? this->GetErr(iX, iY, iT) : 0.);

			dTotal += dVal;
		}

	dat.SetTotal(dTotal);
	return dat;
}

Data1 Data3::GetXY(uint iX, uint iY) const
{
	Data1 dat;
	dat.SetLength(this->GetDepth());

	for(uint iT=0; iT<GetDepth(); ++iT)
	{
		double dVal = GetVal(iX, iY, iT);
		double dErr = GetErr(iX, iY, iT);

		dat.SetX(iT, iT);
		dat.SetXErr(iT, 0.);
		dat.SetY(iT, dVal);
		dat.SetYErr(iT, dErr);
	}

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



//------------------------------------------------------------------------




Data4::Data4(uint iW, uint iH, uint iD, uint iD2, const double* pDat, const double *pErr)
			: m_iDepth(0), m_iDepth2(0),
			  m_dTotal(0.),
			  m_dMin(std::numeric_limits<double>::max()),
			  m_dMax(-std::numeric_limits<double>::max())

{
	this->SetSize(iW, iH, iD, iD2);

	if(pDat)
		this->SetVals(pDat, pErr);

	SetXRange(0., m_iWidth-1);
	SetYRange(0., m_iHeight-1);
}

void Data4::SetSize(uint iWidth, uint iHeight, uint iDepth, uint iDepth2)
{
	if(m_iWidth==iWidth && m_iHeight==iHeight &&
		m_iDepth==iDepth && m_iDepth2==iDepth2)
		return;

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iDepth = iDepth;
	m_iDepth2 = iDepth2;

	m_vecVals.resize(m_iWidth * m_iHeight * m_iDepth * m_iDepth2);
	m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth * m_iDepth2);
}

double Data4::GetValRaw(uint iX, uint iY, uint iD, uint iD2) const
{
	return m_vecVals[iD2*m_iDepth*m_iWidth*m_iHeight +
	                 	 	 iD*m_iWidth*m_iHeight +
	                 	 	 iY*m_iWidth + iX];
}

double Data4::GetErrRaw(uint iX, uint iY, uint iD, uint iD2) const
{
	return m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight +
	                 	 	 iD*m_iWidth*m_iHeight +
	                 	 	 iY*m_iWidth + iX];
}

double Data4::GetVal(uint iX, uint iY, uint iD, uint iD2) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetValRaw(iX, iY, iD, iD2);
	return 0.;
}

double Data4::GetErr(uint iX, uint iY, uint iD, uint iD2) const
{
	if(IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
		return GetErrRaw(iX, iY, iD, iD2);
	return 0.;
}


void Data4::SetVal(uint iX, uint iY, uint iD, uint iD2, double dVal)
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

	for(uint iD2=0; iD2<m_iDepth2; ++iD2)
		SetVals(iD2, pDat+iD2*m_iDepth*m_iWidth*m_iHeight, pErr+iD2*m_iDepth*m_iWidth*m_iHeight);
}

void Data4::SetVals(uint iD2, const double *pDat, const double *pErr)
{
	for(uint iD=0; iD<m_iDepth; ++iD)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = pDat ? pDat[iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;
				double dErr = pErr ? pErr[iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] : 0.;

				this->SetVal(iX, iY, iD, iD2, dVal);
				this->SetErr(iX, iY, iD, iD2, dErr);

				m_dTotal += dVal;
			}
}

void Data4::SetErr(uint iX, uint iY, uint iD, uint iD2, double dVal)
{
	m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight +
	          	  	  iD*m_iWidth*m_iHeight +
	          	  	  iY*m_iWidth + iX] = dVal;
}


Data3 Data4::GetVal(uint iD2) const
{
	Data3 dat(m_iWidth, m_iHeight, m_iDepth);
	dat.CopyXYRangeFrom(this);
	dat.CopyRoiFlagsFrom(this);

	for(uint iD=0; iD<m_iDepth; ++iD)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				dat.SetVal(iX, iY, iD, this->GetVal(iX, iY, iD, iD2));
				dat.SetErr(iX, iY, iD, this->GetErr(iX, iY, iD, iD2));
			}

	return dat;
}

Data2 Data4::GetVal(uint iD, uint iD2) const
{
	Data2 dat(m_iWidth, m_iHeight);
	dat.CopyXYRangeFrom(this);
	dat.CopyRoiFlagsFrom(this);

	double dTotal = 0.;
	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
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

Data1 Data4::GetXYSum(uint iD2) const
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



//------------------------------------------------------------------------



void XYRange::SetXRange(double dXMin, double dXMax)
{
	m_dXMin = dXMin;
	m_dXMax = dXMax;
	m_bHasRange = 1;
}

void XYRange::SetYRange(double dYMin, double dYMax)
{
	m_dYMin = dYMin;
	m_dYMax = dYMax;
	m_bHasRange = 1;
}

// pixel -> range point
double XYRange::GetRangeXPos(uint iX) const
{
	return m_dXMin + (m_dXMax-m_dXMin)*double(iX)/double(m_iWidth-1);
}
double XYRange::GetRangeYPos(uint iY) const
{
	return m_dYMin + (m_dYMax-m_dYMin)*double(iY)/double(m_iHeight-1);
}

// range point -> pixel
uint XYRange::GetPixelXPos(double dRangeX) const
{
	int iX = (dRangeX-m_dXMin)/(m_dXMax-m_dXMin) * double(m_iWidth-1);
	if(iX >= m_iWidth) iX = m_iWidth-1;
	if(iX < 0) iX = 0;
	return iX;
}
uint XYRange::GetPixelYPos(double dRangeY) const
{
	int iY = (dRangeY-m_dYMin)/(m_dYMax-m_dYMin) * double(m_iHeight-1);
	if(iY >= m_iHeight) iY = m_iHeight-1;
	if(iY < 0) iY = 0;
	return iY;
}

void XYRange::CopyXYRangeFrom(const XYRange* pRan)
{
	*this = *pRan;
}
