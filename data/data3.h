/**
 * mieze-tool
 * data representation
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_DAT_3__
#define __MIEZE_DAT_3__

#include "data.h"

class Data3 : public DataInterface, public XYRange
{
protected:
	uint m_iDepth;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

	bool m_bUseErrs;

public:
	Data3(uint iW=128, uint iH=128, uint iD=16,
		const double* pDat=0, const double *pErr=0);
	virtual ~Data3() {}
	virtual DataType GetType() const override { return DATA_3D; }

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
	void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts=false);

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;
};

#endif
