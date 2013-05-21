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

#include "../roi/roi.h"

enum DataType
{
	DATA_1D,
	DATA_2D,
	DATA_3D,
	DATA_4D
};


class RoiFlags
{
protected:
	const bool *m_pbGlobalROIActive;
	const Roi *m_pGlobalROI;

public:
	RoiFlags() : m_pbGlobalROIActive(0), m_pGlobalROI(0) {}
	virtual ~RoiFlags() {}

	bool IsRoiActive() const
	{
		if(m_pbGlobalROIActive)
			return *m_pbGlobalROIActive;
		return 0;
	}

	const Roi* GetRoi() const { return m_pGlobalROI; }

	void CopyRoiFlagsFrom(const RoiFlags* pDat)
	{
		this->m_pbGlobalROIActive = pDat->m_pbGlobalROIActive;
		this->m_pGlobalROI = pDat->m_pGlobalROI;
	}

	void SetGlobalROI(const Roi* pROI, const bool* pbROIActive)
	{
		m_pGlobalROI = pROI;
		m_pbGlobalROIActive = pbROIActive;
	}
};


class DataInterface : public RoiFlags
{
public:
	DataInterface() {}
	virtual ~DataInterface() {}
	virtual DataType GetType() const = 0;
};


class XYRange
{
protected:
	uint m_iWidth, m_iHeight;
	double m_dXMin, m_dXMax;
	double m_dYMin, m_dYMax;
	bool m_bHasRange;

public:
	XYRange() : m_iWidth(0), m_iHeight(0),
				m_dXMin(0.), m_dXMax(1.), m_dYMin(0.), m_dYMax(1.),
				m_bHasRange(0)
	{}

	void SetXRange(double dXMin, double dXMax)
	{
		m_dXMin = dXMin;
		m_dXMax = dXMax;

		m_bHasRange = 1;
	}

	void SetYRange(double dYMin, double dYMax)
	{
		m_dYMin = dYMin;
		m_dYMax = dYMax;

		m_bHasRange = 1;
	}

	bool HasRange() const { return m_bHasRange; }

	// pixel -> range point
	double GetRangeXPos(uint iX) const
	{ return m_dXMin + (m_dXMax-m_dXMin)*double(iX)/double(m_iWidth-1);}
	double GetRangeYPos(uint iY) const
	{ return m_dYMin + (m_dYMax-m_dYMin)*double(iY)/double(m_iHeight-1);}

	// range point -> pixel
	uint GetPixelXPos(double dRangeX) const
	{
		int iX = (dRangeX-m_dXMin)/(m_dXMax-m_dXMin) * double(m_iWidth-1);
		if(iX >= m_iWidth) iX = m_iWidth-1;
		if(iX < 0) iX = 0;
		return iX;
	}
	uint GetPixelYPos(double dRangeY) const
	{
		int iY = (dRangeY-m_dYMin)/(m_dYMax-m_dYMin) * double(m_iHeight-1);
		if(iY >= m_iHeight) iY = m_iHeight-1;
		if(iY < 0) iY = 0;
		return iY;
	}

	void CopyXYRangeFrom(const XYRange* pRan)
	{ *this = *pRan; }
};


class Data1 : public DataInterface
{
protected:
	std::vector<double> m_vecValsX, m_vecValsY;
	std::vector<double> m_vecErrsX, m_vecErrsY;

	std::vector<double> m_vecValsXRoiTmp, m_vecValsYRoiTmp;
	std::vector<double> m_vecErrsXRoiTmp, m_vecErrsYRoiTmp;

public:
	Data1(uint uiNum=0, const double* pValsX=0, const double* pValsY=0,
								const double *pErrsY=0, const double *pErrsX=0);
	virtual ~Data1() {}
	virtual DataType GetType() const { return DATA_1D; }

	double GetX(uint iX) const { return m_vecValsX[iX]; }
	double GetY(uint iY) const { return m_vecValsY[iY]; }
	double GetXErr(uint iX) const { return m_vecErrsX[iX]; }
	double GetYErr(uint iY) const { return m_vecErrsY[iY]; }

	void SetX(uint iX, double dVal) { m_vecValsX[iX] = dVal; }
	void SetY(uint iY, double dVal) { m_vecValsY[iY] = dVal; }
	void SetXErr(uint iX, double dVal) { m_vecErrsX[iX] = dVal; }
	void SetYErr(uint iY, double dVal) { m_vecErrsY[iY] = dVal; }

	uint GetLength() const { return m_vecValsX.size(); }

	// all points
	const std::vector<double>& GetXRaw() const { return m_vecValsX; }
	const std::vector<double>& GetYRaw() const { return m_vecValsY; }
	const std::vector<double>& GetXErrRaw() const { return m_vecErrsX; }
	const std::vector<double>& GetYErrRaw() const { return m_vecErrsY; }

	// only points in roi, all points of no roi active
	void GetData(const std::vector<double> **pvecX, const std::vector<double> **pvecY,
				const std::vector<double> **pvecYErr=0, const std::vector<double> **pvecXErr=0);

	void GetXMinMax(double& dXMin, double& dXMax) const;
	void GetYMinMax(double& dYMin, double& dYMax) const;

	void clear()
	{
		m_vecValsX.clear();
		m_vecValsY.clear();
		m_vecErrsX.clear();
		m_vecErrsY.clear();
	}

	void SetLength(uint uiLen)
	{
		m_vecValsX.resize(uiLen);
		m_vecValsY.resize(uiLen);

		m_vecErrsX.resize(uiLen);
		m_vecErrsY.resize(uiLen);
	}

	template<typename T=double>
	void ToArray(T* pX, T* pY, T *pYErr, T *pXErr=0)
	{
		for(uint i=0; i<GetLength(); ++i)
		{
			if(pX) pX[i] = T(GetX(i));
			if(pY) pY[i] = T(GetY(i));
			if(pYErr) pYErr[i] = T(GetYErr(i));
			if(pXErr) pXErr[i] = T(GetXErr(i));
		}
	}

	double SumY() const
	{
		double dTotal = 0.;
		for(uint i=0; i<GetLength(); ++i)
			dTotal += GetY(i);
		return dTotal;
	}
};


class Data2 : public DataInterface, public XYRange
{
protected:
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data2(uint iW=128, uint iH=128,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data2() {}
	virtual DataType GetType() const { return DATA_2D; }

	void SetZero();

	uint GetWidth() const { return m_iWidth; }
	uint GetHeight() const { return m_iHeight; }

	void SetSize(uint iWidth, uint iHeight)
	{
		if(m_iWidth==iWidth && m_iHeight==iHeight)
			return;

		m_iWidth = iWidth;
		m_iHeight = iHeight;

		m_vecVals.resize(m_iWidth * m_iHeight);
		m_vecErrs.resize(m_iWidth * m_iHeight);
	}

	double GetVal(uint iX, uint iY) const
	{ return m_vecVals[iY*m_iWidth + iX]; }
	double GetErr(uint iX, uint iY) const
	{ return m_vecErrs[iY*m_iWidth + iX]; }

	void SetVal(uint iX, uint iY, double dVal)
	{
		m_vecVals[iY*m_iWidth + iX] = dVal;
		m_dMin = std::min(m_dMin, dVal);
		m_dMax = std::max(m_dMax, dVal);
	}
	void SetErr(uint iX, uint iY, double dVal)
	{ m_vecErrs[iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data1 SumY() const;
};



class Data3 : public DataInterface, public XYRange
{
protected:
	uint m_iDepth;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data3(uint iW=128, uint iH=128, uint iD=16,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data3() {}
	virtual DataType GetType() const { return DATA_3D; }

	uint GetWidth() const { return m_iWidth; }
	uint GetHeight() const { return m_iHeight; }
	uint GetDepth() const { return m_iDepth; }

	void SetZero();

	void SetSize(uint iWidth, uint iHeight, uint iDepth)
	{
		if(m_iWidth==iWidth && m_iHeight==iHeight && m_iDepth==iDepth)
			return;

		m_iWidth = iWidth;
		m_iHeight = iHeight;
		m_iDepth = iDepth;

		m_vecVals.resize(m_iWidth * m_iHeight * m_iDepth);
		m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth);
	}

	double GetVal(uint iX, uint iY, uint iT) const
	{ return m_vecVals[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }
	double GetErr(uint iX, uint iY, uint iT) const
	{ return m_vecErrs[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }

	void SetVal(uint iX, uint iY, uint iT, double dVal)
	{
		m_vecVals[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal;
		m_dMin = std::min(m_dMin, dVal);
		m_dMax = std::max(m_dMax, dVal);
	}
	void SetErr(uint iX, uint iY, uint iT, double dVal)
	{ m_vecErrs[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data2 GetVal(uint iT) const;
	Data1 GetXY(uint iX, uint iY) const;
	Data1 GetXYSum() const;

	void Add(const Data3& dat);
};

class Data4 : public DataInterface, public XYRange
{
protected:
	uint m_iDepth, m_iDepth2;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data4(uint iW=128, uint iH=128, uint iD=16, uint iD2=6,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data4() {}
	virtual DataType GetType() const { return DATA_4D; }

	uint GetWidth() const { return m_iWidth; }
	uint GetHeight() const { return m_iHeight; }
	uint GetDepth() const { return m_iDepth; }
	uint GetDepth2() const { return m_iDepth2; }

	void SetSize(uint iWidth, uint iHeight, uint iDepth, uint iDepth2)
	{
		if(m_iWidth==iWidth && m_iHeight==iHeight && m_iDepth==iDepth && m_iDepth2==iDepth2)
			return;

		m_iWidth = iWidth;
		m_iHeight = iHeight;
		m_iDepth = iDepth;
		m_iDepth2 = iDepth2;

		m_vecVals.resize(m_iWidth * m_iHeight * m_iDepth * m_iDepth2);
		m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth * m_iDepth2);
	}

	double GetVal(uint iX, uint iY, uint iD, uint iD2) const
	{ return m_vecVals[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }
	double GetErr(uint iX, uint iY, uint iD, uint iD2) const
	{ return m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }

	void SetVal(uint iX, uint iY, uint iD, uint iD2, double dVal);
	void SetErr(uint iX, uint iY, uint iD, uint iD2, double dVal)
	{ m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);
	void SetVals(uint iD2, const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data3 GetVal(uint iD2) const;
	Data2 GetVal(uint iD, uint iD2) const;
	Data1 GetXYSum(uint iD2) const;
};

#endif
