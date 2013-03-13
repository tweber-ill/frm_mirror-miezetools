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

enum DataType
{
	DATA_1D,
	DATA_2D
};


class DataInterface
{
public:
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

	void SetLength(uint uiLen)
	{
		m_vecValsX.resize(uiLen);
		m_vecValsY.resize(uiLen);

		m_vecErrsX.resize(uiLen);
		m_vecErrsY.resize(uiLen);
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
	Data2(unsigned int iW=128, unsigned int iH=128, const double* pDat=0, const double *pErr=0);
	virtual ~Data2() {}
	virtual DataType GetType() { return DATA_2D; }

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
};

#endif
