/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data4.h"

#include "../helper/math.h"
#include <limits>
#include <boost/algorithm/minmax_element.hpp>


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

void Data4::RecalcMinMaxTotal()
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iD2=0; iD2<m_iDepth2; ++iD2)
		for(uint iD=0; iD<m_iDepth; ++iD)
			for(uint iY=0; iY<m_iHeight; ++iY)
				for(uint iX=0; iX<m_iWidth; ++iX)
				{
					double dVal = this->GetValRaw(iX, iY, iD, iD2);
					m_dTotal += dVal;

					m_dMin = std::min(m_dMin, dVal);
					m_dMax = std::max(m_dMax, dVal);
				}
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

	double dTotal = 0.;
	for(uint iD=0; iD<m_iDepth; ++iD)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = this->GetValRaw(iX, iY, iD, iD2);
				double dErr = this->GetErrRaw(iX, iY, iD, iD2);

				dat.SetVal(iX, iY, iD, dVal);
				dat.SetErr(iX, iY, iD, dErr);

				dTotal += dVal;
			}

	dat.SetTotal(dTotal);
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
				dVal = this->GetValRaw(iX, iY, iD, iD2);
				dErr = this->GetErrRaw(iX, iY, iD, iD2);
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

	uint iYStart=0, iYEnd=GetHeight();
	uint iXStart=0, iXEnd=GetWidth();

	if(IsRoiActive(0))
	{
		BoundingRect br = m_roi.GetBoundingRect();
		iXStart = GetPixelXPos(br.bottomleft[0]);
		iYStart = GetPixelYPos(br.bottomleft[1]);

		iXEnd = GetPixelXPos(br.topright[0]);
		iYEnd = GetPixelYPos(br.topright[1]);

		if(iXEnd > GetWidth()) iXEnd = GetWidth();
		if(iYEnd > GetHeight()) iYEnd = GetHeight();
		if(iXStart > GetWidth()) iXStart = GetWidth();
		if(iYStart > GetHeight()) iYStart = GetHeight();
	}


	double* pMem = new double[dat.GetLength()*2];
	double* dSum = pMem;
	double* dErrSum = pMem + dat.GetLength();

	for(uint iT=0; iT<GetDepth(); ++iT)
		dSum[iT] = dErrSum[iT] = 0.;

	for(uint iY=iYStart; iY<iYEnd; ++iY)
	{
		for(uint iX=iXStart; iX<iXEnd; ++iX)
		{
			if(!IsInsideRoi(GetRangeXPos(iX), GetRangeYPos(iY)))
				continue;

			for(uint iT=0; iT<GetDepth(); ++iT)
			{
				dSum[iT] += GetValRaw(iX, iY, iT, iD2);
				dErrSum[iT] += GetErrRaw(iX, iY, iT, iD2)*GetErrRaw(iX, iY, iT, iD2);
			}
		}
	}

	for(uint iT=0; iT<GetDepth(); ++iT)
	{
		dat.SetX(iT, iT);
		dat.SetXErr(iT, 0.);
		dat.SetY(iT, dSum[iT]);
		dat.SetYErr(iT, sqrt(dErrSum[iT]) /*sqrt(dSum[iT])*/);
	}

	delete[] pMem;
	return dat;
}

Data1 Data4::GetXYD2(uint iX, uint iY, uint iD2) const
{
	Data1 dat;
	dat.SetLength(this->GetDepth());

	for(uint iT=0; iT<GetDepth(); ++iT)
	{
		double dVal = GetVal(iX, iY, iT, iD2);
		double dErr = GetErr(iX, iY, iT, iD2);

		dat.SetX(iT, iT);
		dat.SetXErr(iT, 0.);
		dat.SetY(iT, dVal);
		dat.SetYErr(iT, dErr);
	}

	return dat;
}


void Data4::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	std::vector<double> vecVals = m_vecVals;
	std::vector<double> vecErrs = m_vecErrs;

	m_vecVals.resize(m_iDepth2*m_iDepth*iNewWidth*iNewHeight);
	m_vecErrs.resize(m_iDepth2*m_iDepth*iNewWidth*iNewHeight);

	double dAreaFactor = double(m_iWidth*m_iHeight)/double(iNewWidth*iNewHeight);

	for(int iF=0; iF<m_iDepth2; ++iF)
		for(int iT=0; iT<m_iDepth; ++iT)
			for(int iY=0; iY<iNewHeight; ++iY)
				for(int iX=0; iX<iNewWidth; ++iX)
				{
					double dOldY0 = ((iY+0.5)*double(m_iHeight))/double(iNewHeight) - 0.5;
					double dOldX0 = ((iX+0.5)*double(m_iWidth))/double(iNewWidth) - 0.5;

					int iX0 = int(dOldX0);
					int iY0 = int(dOldY0);
					int iX1 = iX0+1;
					int iY1 = iY0+1;

					if(iX0<0) iX0=0;
					if(iY0<0) iY0=0;
					if(iX1<0) iX1=0;
					if(iY1<0) iY1=0;

					if(iX0>=m_iWidth) iX0=m_iWidth-1;
					if(iY0>=m_iHeight) iY0=m_iHeight-1;
					if(iX1>=m_iWidth) iX1=m_iWidth-1;
					if(iY1>=m_iHeight) iY1=m_iHeight-1;

					double dX = (dOldX0 - double(iX0));
					double dY = (dOldY0 - double(iY0));

					if(dX<0.) dX=0.; if(dX>1.) dX=1.;
					if(dY<0.) dY=0.; if(dY>1.) dY=1.;


					const uint iOldIdx = iF*m_iDepth*m_iWidth*m_iHeight + iT*m_iWidth*m_iHeight;

					double dx0y0 = vecVals[iOldIdx + iY0*m_iWidth + iX0];
					double dx0y1 = vecVals[iOldIdx + iY1*m_iWidth + iX0];
					double dx1y0 = vecVals[iOldIdx + iY0*m_iWidth + iX1];
					double dx1y1 = vecVals[iOldIdx + iY1*m_iWidth + iX1];

					double dx0y0_err = vecErrs[iOldIdx + iY0*m_iWidth + iX0];
					double dx0y1_err = vecErrs[iOldIdx + iY1*m_iWidth + iX0];
					double dx1y0_err = vecErrs[iOldIdx + iY0*m_iWidth + iX1];
					double dx1y1_err = vecErrs[iOldIdx + iY1*m_iWidth + iX1];


					const uint iNewIdx = iF*m_iDepth*iNewWidth*iNewHeight + iT*iNewWidth*iNewHeight + iY*iNewWidth + iX;

					m_vecVals[iNewIdx] = bilinear_interp<double>(dx0y0, dx1y0, dx0y1, dx1y1, dX, dY);
					m_vecErrs[iNewIdx] = bilinear_interp<double>(dx0y0_err, dx1y0_err, dx0y1_err, dx1y1_err, dX, dY);

					if(bKeepTotalCounts)
					{
						m_vecVals[iNewIdx] *= dAreaFactor;
						m_vecErrs[iNewIdx] *= dAreaFactor;
					}
				}


	//m_dXMax = iNewWidth-1;
	//m_dYMax = iNewHeight-1;

	m_iWidth = iNewWidth;
	m_iHeight = iNewHeight;
}


bool Data4::LoadXML(Xml& xml, Blob& blob, const std::string& strBase)
{
	LoadRangeXml(xml, strBase);
	m_iDepth = xml.Query<unsigned int>((strBase+"depth").c_str(), 0);
	m_iDepth2 = xml.Query<unsigned int>((strBase+"depth2").c_str(), 0);

	m_roi.LoadXML(xml, strBase);
	m_antiroi.LoadXML(xml, strBase);

	unsigned int uiCnt = m_iWidth*m_iHeight*m_iDepth*m_iDepth2;
	m_vecVals.resize(uiCnt);
	m_vecErrs.resize(uiCnt);

	std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	load_xml_vecs(2, vecs, strs, xml, strBase, blob);

	m_dMin = xml.Query<double>((strBase+"min").c_str(), 0.);
	m_dMax = xml.Query<double>((strBase+"max").c_str(), 0.);
	m_dTotal = xml.Query<double>((strBase+"total").c_str(), 0.);

	return 1;
}

bool Data4::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	const bool bSaveInBlob = (m_vecVals.size() > BLOB_SIZE);

	const std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	save_xml_vecs(2, vecs, strs, ostr, ostrBlob, bSaveInBlob);

	ostr << "<min> " << m_dMin << " </min>\n";
	ostr << "<max> " << m_dMax << " </max>\n";
	ostr << "<total> " << m_dTotal << " </total>\n";
	ostr << "<depth> " << m_iDepth << " </depth>\n";
	ostr << "<depth2> " << m_iDepth2 << " </depth2>\n";

	SaveRangeXml(ostr);

	if(m_roi.GetNumElements())
		m_roi.SaveXML(ostr);
	if(m_antiroi.GetNumElements())
		m_antiroi.SaveXML(ostr);

	return 1;
}
