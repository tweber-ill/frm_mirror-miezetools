/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_DAT__
#define __MIEZE_DAT__

typedef unsigned int uint;

#include<vector>

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

#endif
