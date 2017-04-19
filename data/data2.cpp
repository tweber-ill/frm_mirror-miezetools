/**
 * mieze-tool
 * data representation
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-mar-2013
 * @license GPLv3
 */

#include "data2.h"

#include "../tlibs/math/math.h"
#include <limits>
#include <boost/algorithm/minmax_element.hpp>


Data2::Data2(uint iW, uint iH, const double* pDat, const double *pErr)
			: DataInterface(), m_dTotal(0.),
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

void Data2::Add(const Data2& dat)
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			const double dNewVal = GetValRaw(iX, iY)+dat.GetValRaw(iX, iY);
			const double dNewErr = GetErrRaw(iX, iY)+dat.GetErrRaw(iX, iY);

			SetVal(iX, iY, dNewVal);
			SetErr(iX, iY, dNewErr);

			m_dTotal += dNewVal;
		}
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

double Data2::GetTotalInROI() const
{
	double dTotal = 0.;

	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
			dTotal += GetVal(iX, iY);

	return dTotal;
}

void Data2::RecalcMinMaxTotal()
{
	m_dMin = std::numeric_limits<double>::max();
	m_dMax = -m_dMin;
	m_dTotal = 0.;

	for(uint iY=0; iY<m_iHeight; ++iY)
		for(uint iX=0; iX<m_iWidth; ++iX)
		{
			double dVal = this->GetValRaw(iX, iY);
			m_dTotal += dVal;

			m_dMin = std::min(m_dMin, dVal);
			m_dMax = std::max(m_dMax, dVal);
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

	dat1.CopyParamMapsFrom(this);
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

void Data2::ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts)
{
	std::vector<double> vecVals = m_vecVals;
	std::vector<double> vecErrs = m_vecErrs;

	m_vecVals.resize(iNewWidth*iNewHeight);
	m_vecErrs.resize(iNewWidth*iNewHeight);

	double dAreaFactor = double(m_iWidth*m_iHeight)/double(iNewWidth*iNewHeight);

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

			double dx0y0 = vecVals[iY0*m_iWidth + iX0];
			double dx0y1 = vecVals[iY1*m_iWidth + iX0];
			double dx1y0 = vecVals[iY0*m_iWidth + iX1];
			double dx1y1 = vecVals[iY1*m_iWidth + iX1];

			double dx0y0_err = vecErrs[iY0*m_iWidth + iX0];
			double dx0y1_err = vecErrs[iY1*m_iWidth + iX0];
			double dx1y0_err = vecErrs[iY0*m_iWidth + iX1];
			double dx1y1_err = vecErrs[iY1*m_iWidth + iX1];

			m_vecVals[iY*iNewWidth + iX] = tl::bilinear_interp<double>(dx0y0, dx1y0, dx0y1, dx1y1, dX, dY);
			m_vecErrs[iY*iNewHeight + iX] = tl::bilinear_interp<double>(dx0y0_err, dx1y0_err, dx0y1_err, dx1y1_err, dX, dY);

			if(bKeepTotalCounts)
			{
				m_vecVals[iY*iNewWidth + iX] *= dAreaFactor;
				m_vecErrs[iY*iNewWidth + iX] *= dAreaFactor;
			}
		}

	// TODO: handle range correctly
	//if(!m_bHasRange)
	{
		m_dXMax = iNewWidth-1;
		m_dYMax = iNewHeight-1;

		m_iWidth = iNewWidth;
		m_iHeight = iNewHeight;
	}

	RecalcMinMaxTotal();
}


bool Data2::LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase)
{
	LoadRangeXml(xml, strBase);
	m_roi.LoadXML(xml, strBase);
	m_antiroi.LoadXML(xml, strBase);

	unsigned int uiCnt = m_iWidth*m_iHeight;
	m_vecVals.resize(uiCnt);
	m_vecErrs.resize(uiCnt);

	std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	load_xml_vecs(2, vecs, strs, xml, strBase, blob);

	m_dMin = xml.Query<double>((strBase+"min").c_str(), 0.);
	m_dMax = xml.Query<double>((strBase+"max").c_str(), 0.);
	m_dTotal = xml.Query<double>((strBase+"total").c_str(), 0.);


	return DataInterface::LoadXML(xml, blob, strBase);;
}

bool Data2::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	const bool bSaveInBlob = (m_vecVals.size() > BLOB_SIZE);

	const std::vector<double>* vecs[] = {&m_vecVals, &m_vecErrs};
	std::string strs[] = {"vals", "errs"};

	save_xml_vecs(2, vecs, strs, ostr, ostrBlob, bSaveInBlob);

	ostr << "<min> " << m_dMin << " </min>\n";
	ostr << "<max> " << m_dMax << " </max>\n";
	ostr << "<total> " << m_dTotal << " </total>\n";

	SaveRangeXml(ostr);

	if(m_roi.GetNumElements())
		m_roi.SaveXML(ostr);
	if(m_antiroi.GetNumElements())
		m_antiroi.SaveXML(ostr);

	return DataInterface::SaveXML(ostr, ostrBlob);
}
