/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#include "data.h"

void load_xml_vecs(unsigned int iNumVecs,
						std::vector<double>** pvecs,
						const std::string* pstrs,
						Xml& xml,
						const std::string& strBase,
						Blob& blob)
{
	bool bInBlob = xml.Query<bool>((strBase + "in_blob").c_str(), 0);

	for(unsigned int iObj=0; iObj<iNumVecs; ++iObj)
	{
		if(bInBlob)
		{
			bool bHasBlobIdx = 0;
			qint64 iBlobIdx = xml.Query<qint64>((strBase + "blob_" + pstrs[iObj]).c_str(), 0, &bHasBlobIdx);
			if(bHasBlobIdx)
				blob.copy<double>(iBlobIdx, qint64(pvecs[iObj]->size()), pvecs[iObj]->begin());
			else
				std::cerr << "Error: Blob usage enabled, but no blob index given!"
						  << std::endl;
		}
		else
		{
			std::istringstream istr(xml.QueryString((strBase + pstrs[iObj]).c_str(), ""));

			for(unsigned int i=0; i<pvecs[iObj]->size(); ++i)
				istr >> (*pvecs[iObj])[i];
		}
	}
}

void save_xml_vecs(unsigned int iNumVecs,
					const std::vector<double>** pvecs,
					const std::string* pstrs,
					std::ostream& ostr,
					std::ostream& ostrBlob,
					bool bSaveInBlob)
{
	if(iNumVecs==0) return;

	ostr << "<in_blob> " << bSaveInBlob << " </in_blob>\n";

	for(unsigned int iObj=0; iObj<iNumVecs; ++iObj)
	{
		if(bSaveInBlob)
		{
			qint64 iBlobIdx = ostrBlob.tellp();
			for(double d : *pvecs[iObj])
				ostrBlob.write((char*)&d, sizeof(d));

			ostr << "<" << "blob_" << pstrs[iObj] << "> ";
			ostr << iBlobIdx;
			ostr << " </" << "blob_" << pstrs[iObj] << ">\n";
		}
		else
		{
			ostr << "<" << pstrs[iObj] << "> ";
			for(double d : *pvecs[iObj])
				ostr << d << " ";
			ostr << " </" << pstrs[iObj] << ">\n";
		}
	}
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
	// range 0..1
	double dX_Val = double(iX) / double(m_iWidth-1);

	if(m_bXIsLog)
	{
		double dXMin = log10(m_dXMin);
		double dXMax = log10(m_dXMax);
		dX_Val = pow(10., dXMin + dX_Val*(dXMax-dXMin));
	}
	else
		dX_Val = dX_Val * (m_dXMax-m_dXMin) + m_dXMin;

	return dX_Val;
}
double XYRange::GetRangeYPos(uint iY) const
{
	// range 0..1
	double dY_Val = double(iY) / double(m_iHeight-1);

	if(m_bYIsLog)
	{
		double dYMin = log10(m_dYMin);
		double dYMax = log10(m_dYMax);
		dY_Val = pow(10., dYMin + dY_Val*(dYMax-dYMin));
	}
	else
		dY_Val = dY_Val * (m_dYMax-m_dYMin) + m_dYMin;

	return dY_Val;
}

// range point -> pixel
double XYRange::GetPixelXPos(double dRangeX) const
{
	double iX = (dRangeX-m_dXMin)/(m_dXMax-m_dXMin) * (double(m_iWidth)-1.);
	//if(iX >= m_iWidth) iX = m_iWidth-1;
	//if(iX < 0) iX = 0;
	return iX;
}
double XYRange::GetPixelYPos(double dRangeY) const
{
	double iY = (dRangeY-m_dYMin)/(m_dYMax-m_dYMin) * (double(m_iHeight)-1.);
	//if(iY >= m_iHeight) iY = m_iHeight-1;
	//if(iY < 0) iY = 0;
	return iY;
}

void XYRange::CopyXYRangeFrom(const XYRange* pRan)
{
	*((XYRange*)this) = *pRan;
}


bool XYRange::LoadRangeXml(Xml& xml, const std::string& strBase)
{
	m_bHasRange = xml.Query<bool>((strBase+"range/active").c_str(), 0);
	m_iWidth = xml.Query<unsigned int>((strBase+"range/width").c_str(), 0);
	m_iHeight = xml.Query<unsigned int>((strBase+"range/height").c_str(), 0);
	m_dXMin = xml.Query<double>((strBase+"range/min_x").c_str(), 0);
	m_dXMax = xml.Query<double>((strBase+"range/max_x").c_str(), 0);
	m_dYMin = xml.Query<double>((strBase+"range/min_y").c_str(), 0);
	m_dYMax = xml.Query<double>((strBase+"range/max_y").c_str(), 0);
	m_bXIsLog = xml.Query<bool>((strBase+"range/log_x").c_str(), 0);
	m_bYIsLog = xml.Query<bool>((strBase+"range/log_y").c_str(), 0);

	return 1;
}

bool XYRange::SaveRangeXml(std::ostream& ostr) const
{
	ostr << "<range>\n";

	ostr << "<active> " << m_bHasRange << " </active>\n";
	ostr << "<width> " << m_iWidth << " </width>\n";
	ostr << "<height> " << m_iWidth << " </height>\n";
	ostr << "<min_x> " << m_dXMin << " </min_x>\n";
	ostr << "<max_x> " << m_dXMax << " </max_x>\n";
	ostr << "<min_y> " << m_dYMin << " </min_y>\n";
	ostr << "<max_y> " << m_dYMax << " </max_y>\n";
	ostr << "<log_x> " << m_bXIsLog << " </log_x>\n";
	ostr << "<log_y> " << m_bYIsLog << " </log_y>\n";

	ostr << "</range>\n";
	return 1;
}
