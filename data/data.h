/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_DAT__
#define __MIEZE_DAT__

typedef unsigned int uint;
typedef unsigned char uchar;

#include<vector>
#include<algorithm>

#include <boost/numeric/ublas/matrix.hpp>
namespace ublas = boost::numeric::ublas;

#include "../roi/roi.h"
#include "../helper/xml.h"
#include "../helper/blob.h"

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
		Xml& xml,
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

public:
	RoiFlags() {}
	virtual ~RoiFlags() {}

	bool IsRoiActive() const { return m_roi.IsRoiActive(); }
	void SetRoiActive(bool bActive) { m_roi.SetRoiActive(bActive); }

	const Roi& GetRoi() const { return m_roi; }
	Roi& GetRoi() { return m_roi; }

	bool IsInsideRoi(double dX, double dY) const
	{
		return m_roi.IsInside(dX, dY);
	}

	void CopyRoiFlagsFrom(const RoiFlags* pDat)
	{
		this->m_roi = pDat->m_roi;
	}

	void SetROI(const Roi* pROI)
	{
		if(!pROI) return;
		m_roi = *pROI;
		//m_roi.SetRoiActive(1);
	}
};


class DataInterface : public RoiFlags
{
public:
	DataInterface() {}
	virtual ~DataInterface() {}
	virtual DataType GetType() const = 0;

	virtual bool LoadXML(Xml& xml, Blob& blob, const std::string& strBase) { return false; }
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const { return false; }
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
	void SetYRange(double dYMin, double dYMax);;

	void SetXYLog(bool bLogX, bool bLogY) { m_bXIsLog=bLogX; m_bYIsLog=bLogY; }

	bool HasRange() const { return m_bHasRange; }

	// pixel -> range point
	double GetRangeXPos(uint iX) const;
	double GetRangeYPos(uint iY) const;

	// range point -> pixel
	double GetPixelXPos(double dRangeX) const;
	double GetPixelYPos(double dRangeY) const;

	void CopyXYRangeFrom(const XYRange* pRan);

	bool LoadRangeXml(Xml& xml, const std::string& strBase);
	bool SaveRangeXml(std::ostream& ostr) const;
};


#include "data1.h"
#include "data2.h"
#include "data3.h"
#include "data4.h"

#endif
