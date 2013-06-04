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

	bool IsInsideRoi(double dX, double dY) const
	{
		if(!IsRoiActive())
			return 1;

		bool bInGlobalRoi = 0;
		if(m_pbGlobalROIActive && *m_pbGlobalROIActive)
			bInGlobalRoi = m_pGlobalROI->IsInside(dX, dY);

		bool bInLocalRoi = 0;
		// TODO

		return bInGlobalRoi || bInLocalRoi;
	}

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
	uint GetPixelXPos(double dRangeX) const;
	uint GetPixelYPos(double dRangeY) const;

	void CopyXYRangeFrom(const XYRange* pRan);
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

	// only points in roi, all points if no roi active
	void GetData(const std::vector<double> **pvecX, const std::vector<double> **pvecY,
				const std::vector<double> **pvecYErr=0, const std::vector<double> **pvecXErr=0);

	void GetXMinMax(double& dXMin, double& dXMax) const;
	void GetYMinMax(double& dYMin, double& dYMax) const;

	void clear();
	void SetLength(uint uiLen);

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

	double SumY() const;
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

	void SetSize(uint iWidth, uint iHeight);

	double GetValRaw(uint iX, uint iY) const;
	double GetErrRaw(uint iX, uint iY) const;

	double GetVal(uint iX, uint iY) const;
	double GetErr(uint iX, uint iY) const;

	void SetVal(uint iX, uint iY, double dVal);
	void SetErr(uint iX, uint iY, double dVal);
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	void SetMinMax(double dMin, double dMax)  { m_dMin=dMin; m_dMax=dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	void FromMatrix(const ublas::matrix<double>& mat);

	Data1 SumY() const;

	void RecalcMinMaxTotal();
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

	void SetSize(uint iWidth, uint iHeight, uint iDepth);

	double GetValRaw(uint iX, uint iY, uint iT) const;
	double GetErrRaw(uint iX, uint iY, uint iT) const;

	double GetVal(uint iX, uint iY, uint iT) const;
	double GetErr(uint iX, uint iY, uint iT) const;
	void SetVal(uint iX, uint iY, uint iT, double dVal);
	void SetErr(uint iX, uint iY, uint iT, double dVal);
	void SetVals(const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data2 GetVal(uint iT) const;
	Data1 GetXY(uint iX, uint iY) const;
	Data1 GetXYSum() const;

	void Add(const Data3& dat);

	void RecalcMinMaxTotal();
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

	void SetSize(uint iWidth, uint iHeight, uint iDepth, uint iDepth2);

	double GetValRaw(uint iX, uint iY, uint iD, uint iD2) const;
	double GetErrRaw(uint iX, uint iY, uint iD, uint iD2) const;

	double GetVal(uint iX, uint iY, uint iD, uint iD2) const;
	double GetErr(uint iX, uint iY, uint iD, uint iD2) const;

	void SetVal(uint iX, uint iY, uint iD, uint iD2, double dVal);
	void SetErr(uint iX, uint iY, uint iD, uint iD2, double dVal);
	void SetVals(const double *pDat, const double *pErr=0);
	void SetVals(uint iD2, const double *pDat, const double *pErr=0);

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data3 GetVal(uint iD2) const;
	Data2 GetVal(uint iD, uint iD2) const;
	Data1 GetXYSum(uint iD2) const;
	Data1 GetXYD2(uint iX, uint iY, uint iD2) const;

	void RecalcMinMaxTotal();
};

#endif
