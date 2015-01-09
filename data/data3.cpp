/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data3.h"

#include "../tlibs/math/math.h"
#include <limits>
#include <boost/algorithm/minmax_element.hpp>


Data3::Data3(uint iW, uint iH, uint iT, const double* pDat, const double *pErr)
			: DataInterface(),
			  m_iDepth(0),
			  m_dTotal(0.),
			  m_dMin(std::numeric_limits<double>::max()),
			  m_dMax(-std::numeric_limits<double>::max()),
			  m_bUseErrs(pErr!=0)

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

void Data3::RecalcMinMaxTotal()
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iD=0; iD<m_iDepth; ++iD)
		for(uint iY=0; iY<m_iHeight; ++iY)
			for(uint iX=0; iX<m_iWidth; ++iX)
			{
				double dVal = this->GetValRaw(iX, iY, iD);
				m_dTotal += dVal;

				m_dMin = std::min(m_dMin, dVal);
				m_dMax = std::max(m_dMax, dVal);
			}
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
				const double dNewVal = GetValRaw(iX, iY, iT)+dat.GetValRaw(iX, iY, iT);
				const double dNewErr = GetErrRaw(iX, iY, iT)+dat.GetErrRaw(iX, iY, iT);

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
	if(m_bUseErrs)
		m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth);
}

double Data3::GetValRaw(uint iX, uint iY, uint iT) const
{
	return m_vecVals[iT*m_iWidth*m_iHeight +
	                 	 	 iY*m_iWidth + iX];
}
double Data3::GetErrRaw(uint iX, uint iY, uint iT) const
{
	if(m_bUseErrs)
		return m_vecErrs[iT*m_iWidth*m_iHeight +
								 iY*m_iWidth + iX];
	else
		return 0.;
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
	if(m_bUseErrs)
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
	dat.CopyParamMapsFrom(this);
	dat.CopyXYRangeFrom(this);
	dat.CopyRoiFlagsFrom(this);

	double dTotal = 0.;
	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = iT<m_iDepth ? this->GetValRaw(iX, iY, iT) : 0.;

			dat.SetVal(iX, iY, dVal);
			dat.SetErr(iX, iY, iT<m_iDepth ? this->GetErrRaw(iX, iY, iT) : 0.);

			dTotal += dVal;
		}

	dat.SetTotal(dTotal);
	return dat;
}

Data1 Data3::GetXY(uint iX, uint iY) const
{
	Data1 dat;
	dat.CopyParamMapsFrom(this);
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
	dat.CopyParamMapsFrom(this);
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
				dSum[iT] += GetValRaw(iX, iY, iT);
				dErrSum[iT] += GetErrRaw(iX, iY, iT)*GetErrRaw(iX, iY, iT);
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


void Data3::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	std::vector<double> vecVals = m_vecVals;
	std::vector<double> vecErrs;
	if(m_bUseErrs) vecErrs = m_vecErrs;

	m_vecVals.resize(m_iDepth*iNewWidth*iNewHeight);

	if(m_bUseErrs)
		m_vecErrs.resize(m_iDepth*iNewWidth*iNewHeight);

	double dAreaFactor = double(m_iWidth*m_iHeight)/double(iNewWidth*iNewHeight);

	for(int iT=0; iT<int(m_iDepth); ++iT)
		for(int iY=0; iY<int(iNewHeight); ++iY)
			for(int iX=0; iX<int(iNewWidth); ++iX)
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

				if(iX0>=int(m_iWidth)) iX0=int(m_iWidth)-1;
				if(iY0>=int(m_iHeight)) iY0=int(m_iHeight)-1;
				if(iX1>=int(m_iWidth)) iX1=int(m_iWidth)-1;
				if(iY1>=int(m_iHeight)) iY1=int(m_iHeight)-1;

				double dX = (dOldX0 - double(iX0));
				double dY = (dOldY0 - double(iY0));

				if(dX<0.) dX=0.; if(dX>1.) dX=1.;
				if(dY<0.) dY=0.; if(dY>1.) dY=1.;

				const uint iOldIdx = iT*m_iWidth*m_iHeight;

				double dx0y0 = vecVals[iOldIdx + iY0*m_iWidth + iX0];
				double dx0y1 = vecVals[iOldIdx + iY1*m_iWidth + iX0];
				double dx1y0 = vecVals[iOldIdx + iY0*m_iWidth + iX1];
				double dx1y1 = vecVals[iOldIdx + iY1*m_iWidth + iX1];

				double dx0y0_err = 0.;
				double dx0y1_err = 0.;
				double dx1y0_err = 0.;
				double dx1y1_err = 0.;

				if(vecErrs.size())
				{
					dx0y0_err = vecErrs[iOldIdx + iY0*m_iWidth + iX0];
					dx0y1_err = vecErrs[iOldIdx + iY1*m_iWidth + iX0];
					dx1y0_err = vecErrs[iOldIdx + iY0*m_iWidth + iX1];
					dx1y1_err = vecErrs[iOldIdx + iY1*m_iWidth + iX1];
				}

				const uint iNewIdx = iT*iNewWidth*iNewHeight + iY*iNewWidth + iX;

				m_vecVals[iNewIdx] = tl::bilinear_interp<double>(dx0y0, dx1y0, dx0y1, dx1y1, dX, dY);
				if(m_bUseErrs)
					m_vecErrs[iNewIdx] = tl::bilinear_interp<double>(dx0y0_err, dx1y0_err, dx0y1_err, dx1y1_err, dX, dY);

				if(bKeepTotalCounts)
				{
					m_vecVals[iNewIdx] *= dAreaFactor;
					if(m_bUseErrs)
						m_vecErrs[iNewIdx] *= dAreaFactor;
				}
			}

	// TODO: handle range correctly
	m_dXMax = iNewWidth-1;
	m_dYMax = iNewHeight-1;

	m_iWidth = iNewWidth;
	m_iHeight = iNewHeight;

	RecalcMinMaxTotal();
}


bool Data3::LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase)
{
	LoadRangeXml(xml, strBase);
	m_iDepth = xml.Query<unsigned int>((strBase+"depth").c_str(), 0);
	m_bUseErrs = xml.Query<int>((strBase+"use_errs").c_str(), 0);

	m_roi.LoadXML(xml, strBase);
	m_antiroi.LoadXML(xml, strBase);

	unsigned int uiCnt = m_iWidth*m_iHeight*m_iDepth;
	m_vecVals.resize(uiCnt);
	if(m_bUseErrs)
		m_vecErrs.resize(uiCnt);

	std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	load_xml_vecs(m_bUseErrs?2:1, vecs, strs, xml, strBase, blob);

	m_dMin = xml.Query<double>((strBase+"min").c_str(), 0.);
	m_dMax = xml.Query<double>((strBase+"max").c_str(), 0.);
	m_dTotal = xml.Query<double>((strBase+"total").c_str(), 0.);

	return DataInterface::LoadXML(xml, blob, strBase);;
}

bool Data3::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	const bool bSaveInBlob = (m_vecVals.size() > BLOB_SIZE);

	const std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	save_xml_vecs(m_bUseErrs?2:1, vecs, strs, ostr, ostrBlob, bSaveInBlob);

	ostr << "<min> " << m_dMin << " </min>\n";
	ostr << "<max> " << m_dMax << " </max>\n";
	ostr << "<total> " << m_dTotal << " </total>\n";
	ostr << "<depth> " << m_iDepth << " </depth>\n";
	ostr << "<use_errs> " << m_bUseErrs << " </use_errs>\n";

	SaveRangeXml(ostr);

	if(m_roi.GetNumElements())
		m_roi.SaveXML(ostr);
	if(m_antiroi.GetNumElements())
		m_antiroi.SaveXML(ostr);

	return DataInterface::SaveXML(ostr, ostrBlob);
}
