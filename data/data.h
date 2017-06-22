/**
 * mieze-tool
 * data representation
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_DAT__
#define __MIEZE_DAT__

typedef unsigned int uint;
typedef unsigned char uchar;

#include <vector>
#include <algorithm>

#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#include "roi/roi.h"
#include "tlibs/string/string.h"
#include "helper/xml.h"
#include "helper/blob.h"
#include "helper/string_map.h"

enum DataType
{
	DATA_1D,
	DATA_2D,
	DATA_3D,
	DATA_4D
};

class Data1;
class Data2;
class Data3;
class Data4;


// size beyond which session data is saved in the blob
// file rather than the xml
#define BLOB_SIZE 256

extern void load_xml_vecs(unsigned int iNumVecs,
	std::vector<double>** pvecs,
	const std::string* pstrs,
	tl::Xml& xml,
	const std::string& strBase,
	Blob& blob);

extern void save_xml_vecs(unsigned int iNumVecs,
	const std::vector<double>** pvecs,
	const std::string* pstrs,
	std::ostream& ostr,
	std::ostream& ostrBlob,
	bool bSaveInBlob);


class RoiFlags
{
protected:
	Roi m_roi;
	Roi m_antiroi;

public:
	RoiFlags()
	{
		m_roi.SetName("roi");
		m_antiroi.SetName("antiroi");
	}
	virtual ~RoiFlags() {}

	bool IsAnyRoiActive() const
	{
		return m_antiroi.IsRoiActive() || m_roi.IsRoiActive();
	}

	bool IsRoiActive(bool bAntiRoi=0) const
	{
		return (bAntiRoi ? m_antiroi.IsRoiActive() : m_roi.IsRoiActive());
	}

	void SetRoiActive(bool bActive, bool bAntiRoi=0)
	{
		if(bAntiRoi)
			m_antiroi.SetRoiActive(bActive);
		else
			m_roi.SetRoiActive(bActive);
	}

	const Roi& GetRoi(bool bAntiRoi=0) const
	{
		if(bAntiRoi)
			return m_antiroi;
		return m_roi;
	}
	Roi& GetRoi(bool bAntiRoi=0)
	{
		if(bAntiRoi)
			return m_antiroi;
		return m_roi;
	}

	bool IsInsideRoi(double dX, double dY) const
	{
		bool bInsideROI = true;
		if(m_roi.IsRoiActive())
			bInsideROI = m_roi.IsInside(dX, dY);

		bool bOutsideAntiROI = true;
		if(m_antiroi.IsRoiActive())
			bOutsideAntiROI = !m_antiroi.IsInside(dX, dY);

		return bInsideROI && bOutsideAntiROI;
	}

	void CopyRoiFlagsFrom(const RoiFlags* pDat)
	{
		this->m_roi = pDat->m_roi;
		this->m_antiroi = pDat->m_antiroi;
	}

	void SetROI(const Roi* pROI, bool bAntiRoi=0)
	{
		if(!pROI) return;

		if(bAntiRoi)
			m_antiroi = *pROI;
		else
			m_roi = *pROI;
	}
};


class DataInterface : public RoiFlags
{
protected:
	StringMap m_mapDynData;
	StringMap m_mapStatData;

public:
	DataInterface() : RoiFlags() {}
	virtual ~DataInterface() {}
	virtual DataType GetType() const = 0;

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase);
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const;

	StringMap& GetParamMapDyn() { return m_mapDynData; }
	const StringMap& GetParamMapDyn() const { return m_mapDynData; }
	void SetParamMapDyn(const StringMap& mapParam) { m_mapDynData = mapParam; }

	const StringMap& GetParamMapStat() const { return m_mapStatData; }
	void SetParamMapStat(const StringMap& mapParam) { m_mapStatData = mapParam; }

	void CopyParamMapsFrom(const DataInterface* pInt)
	{
		this->m_mapStatData = pInt->m_mapStatData;
		this->m_mapDynData = pInt->m_mapDynData;
	};
};


class XYRange
{
protected:
	uint m_iWidth, m_iHeight;
	double m_dXMin, m_dXMax;
	double m_dYMin, m_dYMax;
	bool m_bHasRange;
	bool m_bXIsLog, m_bYIsLog;

public:
	XYRange() : m_iWidth(0), m_iHeight(0),
		m_dXMin(0.), m_dXMax(1.), m_dYMin(0.), m_dYMax(1.),
		m_bHasRange(0), m_bXIsLog(0), m_bYIsLog(0)
	{}

	void SetXRange(double dXMin, double dXMax);
	void SetYRange(double dYMin, double dYMax);

	double GetXRangeMin() const { return m_dXMin; }
	double GetXRangeMax() const { return m_dXMax; }
	double GetYRangeMin() const { return m_dYMin; }
	double GetYRangeMax() const { return m_dYMax; }

	void SetXYLog(bool bLogX, bool bLogY) { m_bXIsLog=bLogX; m_bYIsLog=bLogY; }

	bool HasRange() const { return m_bHasRange; }

	// pixel -> range point
	double GetRangeXPos(uint iX) const;
	double GetRangeYPos(uint iY) const;

	// range point -> pixel
	double GetPixelXPos(double dRangeX) const;
	double GetPixelYPos(double dRangeY) const;

	void CopyXYRangeFrom(const XYRange* pRan);

	bool LoadRangeXml(tl::Xml& xml, const std::string& strBase);
	bool SaveRangeXml(std::ostream& ostr) const;
};


#include "data1.h"
#include "data2.h"
#include "data3.h"
#include "data4.h"

#endif
