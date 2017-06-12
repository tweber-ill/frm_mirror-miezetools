/**
 * mieze-tool
 * data representation
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 08-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_DAT_1__
#define __MIEZE_DAT_1__

#include "data.h"

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
	//Data1(const Data1& dat);
	//const Data1& operator=(const Data1&);

	virtual ~Data1() {}
	virtual DataType GetType() const override { return DATA_1D; }

	const double* GetXPtr() const { return m_vecValsX.data(); }
	const double* GetYPtr() const { return m_vecValsY.data(); }
	const double* GetXErrPtr() const { return m_vecErrsX.data(); }
	const double* GetYErrPtr() const { return m_vecErrsY.data(); }

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
	void GetXErrMinMax(double& dXMin, double& dXMax) const;
	void GetYErrMinMax(double& dYMin, double& dYMax) const;

	void clear();
	void SetLength(uint uiLen);

	template<typename T=double>
	void ToArray(T* pX, T* pY, T *pYErr, T *pXErr=0) const
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

	virtual bool LoadXML(tl::Xml& xml, Blob& blob, const std::string& strBase) override;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob, bool bSaveActualData) const;
	virtual bool SaveXML(std::ostream& ostr, std::ostream& ostrBlob) const override
	{ return SaveXML(ostr, ostrBlob, 1); }
};

#endif
