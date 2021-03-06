/**
 * mieze-tool
 * data representation
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_DAT_4__
#define __MIEZE_DAT_4__

#include "data.h"

class Data4 : public DataInterface, public XYRange
{
protected:
	uint m_iDepth, m_iDepth2;
	std::vector<double> m_vecVals;
	std::vector<double> m_vecErrs;
	double m_dMin, m_dMax;
	double m_dTotal;	// sum of all values

	std::vector<double> m_vecPhases;
	bool m_bUseErrs;

public:
	Data4(uint iW=128, uint iH=128, uint iD=16, uint iD2=6,
				const double* pDat=0, const double *pErr=0);
	virtual ~Data4() {}
	virtual DataType GetType() const override { return DATA_4D; }

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

	void SetHasPhases(bool bHas) { m_vecPhases.resize(bHas ? m_iDepth2 : 0);  }
	bool HasPhases() const { return (m_vecPhases.size()==m_iDepth2); }
	const std::vector<double>& GetPhases() const { return m_vecPhases; }
	void SetPhases(const std::vector<double>& vec) { m_vecPhases = vec; }

	double GetMin() const { return m_dMin; }
	double GetMax() const { return m_dMax; }

	double GetTotal() const { return m_dTotal; }
	void SetTotal(double dTot) { m_dTotal = dTot; }

	Data3 GetVal(uint iD2) const;
	Data2 GetVal(uint iD, uint iD2) const;
	Data1 GetXYSum(uint iD2) const;
	Data1 GetXYD2(uint iX, uint iY, uint iD2) const;

	void RecalcMinMaxTotal();
	void ChangeResolution(unsigned int iNewWidth, unsigned int iNewHeight, bool bKeepTotalCounts=false);

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override;
};

#endif
