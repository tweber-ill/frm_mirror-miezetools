/**
 * loading of nicos data files
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 25-04-2013
 * @license GPLv3
 */

#ifndef __LOAD_NICOS__
#define __LOAD_NICOS__

#include "loadtxt.h"
#include <vector>
#include <string>

class NicosData
{
	private:
		static void FilterComments(tl::LoadTxt::t_mapComm& mapComm);

	protected:
		const tl::LoadTxt& m_data;
		bool m_bOk;

		std::vector<std::string> m_vecColNames;
		std::vector<std::string> m_vecColUnits;

	public:
		NicosData(const tl::LoadTxt& data);
		virtual ~NicosData();

		bool IsOk() const { return m_bOk; }

		bool GetString(const std::string& strKey, std::string& strVal) const
		{ return m_data.GetMapString(strKey, strVal); }
		bool GetDouble(const std::string& strKey, double& dVal) const
		{ return m_data.GetMapVal<double>(strKey, dVal); }

		const tl::LoadTxt& GetRawData() const { return m_data; }
		const std::string& GetFileName() const { return m_data.GetFileName(); }

		unsigned int GetDim(void) const { return m_data.GetColLen(); }
		unsigned int GetColCnt() const { return m_data.GetColCnt(); }
		const double* GetColumn(unsigned int iIdx) const { return m_data.GetColumn(iIdx); }
		const std::string& GetColName(unsigned int iIdx) const;
		const std::vector<std::string>& GetColNames() const { return m_vecColNames; }
		const std::string& GetColUnit(unsigned int iIdx) const;

		int GetColIdx(const std::string& strName) const;

		std::string TryFindScanVar(tl::LoadTxt::t_mapComm& mapComm) const;
};


#endif
