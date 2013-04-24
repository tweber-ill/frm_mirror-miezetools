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
	virtual DataType GetType() = 0;
};


class Data1 : public DataInterface
{
protected:
	std::vector<double> m_vecValsX, m_vecValsY;
	std::vector<double> m_vecErrsX, m_vecErrsY;

public:
	Data1(unsigned int uiNum=0, const double* pValsX=0, const double* pValsY=0,
													const double *pErrsY=0, const double *pErrsX=0);
	virtual ~Data1() {}
	virtual DataType GetType() { return DATA_1D; }

	double GetX(uint iX) const { return m_vecValsX[iX]; }
	double GetY(uint iY) const { return m_vecValsY[iY]; }
	double GetXErr(uint iX) const { return m_vecErrsX[iX]; }
	double GetYErr(uint iY) const { return m_vecErrsY[iY]; }

	void SetX(uint iX, double dVal) { m_vecValsX[iX] = dVal; }
	void SetY(uint iY, double dVal) { m_vecValsY[iY] = dVal; }
	void SetXErr(uint iX, double dVal) { m_vecErrsX[iX] = dVal; }
	void SetYErr(uint iY, double dVal) { m_vecErrsY[iY] = dVal; }

	unsigned int GetLength() const { return m_vecValsX.size(); }

	const std::vector<double>& GetX() const { return m_vecValsX; }
	const std::vector<double>& GetY() const { return m_vecValsY; }
	const std::vector<double>& GetXErr() const { return m_vecErrsX; }
	const std::vector<double>& GetYErr() const { return m_vecErrsY; }

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
		for(unsigned int i=0; i<GetLength(); ++i)
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
		for(unsigned int i=0; i<GetLength(); ++i)
			dTotal += GetY(i);
		return dTotal;
	}
};


class Data2 : public DataInterface
{
protected:
	unsigned int m_iWidth, m_iHeight;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data2(unsigned int iW=128, unsigned int iH=128,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data2() {}
	virtual DataType GetType() { return DATA_2D; }

	void SetZero();

	unsigned int GetWidth() const { return m_iWidth; }
	unsigned int GetHeight() const { return m_iHeight; }

	void SetSize(unsigned int iWidth, unsigned int iHeight)
	{
		if(m_iWidth==iWidth && m_iHeight==iHeight)
			return;

		m_iWidth = iWidth;
		m_iHeight = iHeight;

		m_vecVals.resize(m_iWidth * m_iHeight);
		m_vecErrs.resize(m_iWidth * m_iHeight);
	}

	double GetVal(unsigned int iX, unsigned int iY) const
	{ return m_vecVals[iY*m_iWidth + iX]; }
	double GetErr(unsigned int iX, unsigned int iY) const
	{ return m_vecErrs[iY*m_iWidth + iX]; }

	void SetVal(unsigned int iX, unsigned int iY, double dVal)
	{
		m_vecVals[iY*m_iWidth + iX] = dVal;
		m_dMin = std::min(m_dMin, dVal);
		m_dMax = std::max(m_dMax, dVal);
	}
	void SetErr(unsigned int iX, unsigned int iY, double dVal)
	{ m_vecErrs[iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }
};



class Data3 : public DataInterface
{
protected:
	unsigned int m_iWidth, m_iHeight, m_iDepth;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data3(unsigned int iW=128, unsigned int iH=128, unsigned int iD=16,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data3() {}
	virtual DataType GetType() { return DATA_3D; }

	unsigned int GetWidth() const { return m_iWidth; }
	unsigned int GetHeight() const { return m_iHeight; }
	unsigned int GetDepth() const { return m_iDepth; }

	void SetZero();

	void SetSize(unsigned int iWidth, unsigned int iHeight, unsigned int iDepth)
	{
		if(m_iWidth==iWidth && m_iHeight==iHeight && m_iDepth==iDepth)
			return;

		m_iWidth = iWidth;
		m_iHeight = iHeight;
		m_iDepth = iDepth;

		m_vecVals.resize(m_iWidth * m_iHeight * m_iDepth);
		m_vecErrs.resize(m_iWidth * m_iHeight * m_iDepth);
	}

	double GetVal(unsigned int iX, unsigned int iY, unsigned int iT) const
	{ return m_vecVals[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }
	double GetErr(unsigned int iX, unsigned int iY, unsigned int iT) const
	{ return m_vecErrs[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }

	void SetVal(unsigned int iX, unsigned int iY, unsigned int iT, double dVal)
	{
		m_vecVals[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal;
		m_dMin = std::min(m_dMin, dVal);
		m_dMax = std::max(m_dMax, dVal);
	}
	void SetErr(unsigned int iX, unsigned int iY, unsigned int iT, double dVal)
	{ m_vecErrs[iT*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data2 GetVal(unsigned int iT) const;
	Data1 GetXY(unsigned int iX, unsigned int iY) const;
	Data1 GetXYSum() const;

	void Add(const Data3& dat);
};

class Data4 : public DataInterface
{
protected:
	unsigned int m_iWidth, m_iHeight, m_iDepth, m_iDepth2;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

public:
	Data4(unsigned int iW=128, unsigned int iH=128, unsigned int iD=16, unsigned int iD2=6,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data4() {}
	virtual DataType GetType() { return DATA_4D; }

	unsigned int GetWidth() const { return m_iWidth; }
	unsigned int GetHeight() const { return m_iHeight; }
	unsigned int GetDepth() const { return m_iDepth; }
	unsigned int GetDepth2() const { return m_iDepth2; }

	void SetSize(unsigned int iWidth, unsigned int iHeight, unsigned int iDepth, unsigned int iDepth2)
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

	double GetVal(unsigned int iX, unsigned int iY, unsigned int iD, unsigned int iD2) const
	{ return m_vecVals[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }
	double GetErr(unsigned int iX, unsigned int iY, unsigned int iD, unsigned int iD2) const
	{ return m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX]; }

	void SetVal(unsigned int iX, unsigned int iY, unsigned int iD, unsigned int iD2, double dVal);
	void SetErr(unsigned int iX, unsigned int iY, unsigned int iD, unsigned int iD2, double dVal)
	{ m_vecErrs[iD2*m_iDepth*m_iWidth*m_iHeight + iD*m_iWidth*m_iHeight + iY*m_iWidth + iX] = dVal; }
	void SetVals(const double *pDat, const double *pErr=0);
	void SetVals(unsigned int iD2, const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data3 GetVal(unsigned int iD2) const;
	Data2 GetVal(unsigned int iD, unsigned int iD2) const;
	Data1 GetXYSum(unsigned int iD2) const;
};

#endif
