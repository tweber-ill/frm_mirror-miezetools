/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data1.h"

#include "../helper/math.h"
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

void Data1::GetXErrMinMax(double& dXMin, double& dXMax) const
{
	typedef std::vector<double>::const_iterator t_iter;
	std::pair<t_iter, t_iter> pair =
			boost::minmax_element(m_vecErrsX.begin(), m_vecErrsX.end());

	dXMin = (pair.first==m_vecErrsX.end() ? 0. : *pair.first);
	dXMax = (pair.second==m_vecErrsX.end() ? 0. : *pair.second);
}

void Data1::GetYErrMinMax(double& dYMin, double& dYMax) const
{
	typedef std::vector<double>::const_iterator t_iter;
	std::pair<t_iter, t_iter> pair =
			boost::minmax_element(m_vecErrsY.begin(), m_vecErrsY.end());

	dYMin = (pair.first==m_vecErrsY.end()? 0.: *pair.first);
	dYMax = (pair.second==m_vecErrsY.end()? 0.: *pair.second);
}


bool Data1::LoadXML(Xml& xml, Blob& blob, const std::string& strBase)
{
	unsigned int uiLen = xml.Query<unsigned int>((strBase + "length").c_str(), 0);
	m_vecValsX.resize(uiLen);
	m_vecValsY.resize(uiLen);
	m_vecErrsX.resize(uiLen);
	m_vecErrsY.resize(uiLen);

	std::vector<double>* vecs[] = {&m_vecValsX, &m_vecValsY, &m_vecErrsX, &m_vecErrsY};
	std::string strs[] = {"x", "y", "x_err", "y_err"};

	load_xml_vecs(4, vecs, strs, xml, strBase, blob);

	m_roi.LoadXML(xml, strBase);
	return 1;
}

bool Data1::SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const
{
	ostr << "<length> " << m_vecValsX.size() << "</length>\n";

	const bool bSaveInBlob = (m_vecValsX.size() > BLOB_SIZE);

	const std::vector<double>* vecs[] = {&m_vecValsX, &m_vecValsY, &m_vecErrsX, &m_vecErrsY};
	std::string strs[] = {"x", "y", "x_err", "y_err"};

	save_xml_vecs(4, vecs, strs, ostr, ostrBlob, bSaveInBlob);

	if(m_roi.GetNumElements())
		m_roi.SaveXML(ostr);

	return 1;
}
