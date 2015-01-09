/*
 * mieze-tool
 * data representation
 * @author tweber
 * @date 08-mar-2013
 */

#ifndef __MIEZE_DAT_2__
#define __MIEZE_DAT_2__

#include "data.h"

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
	void Add(const Data2& dat);

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
	double GetTotalInROI() const;

	void FromMatrix(const ublas::matrix<double>& mat);

	Data1 SumY() const;

	void RecalcMinMaxTotal();
	void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts=false);

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase);
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const;
};

#endif
