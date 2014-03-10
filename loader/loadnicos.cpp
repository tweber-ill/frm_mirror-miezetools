/*
 * loading of nicos data files
 * @author tweber
 * @data 25-04-2013
 */

#include "loadnicos.h"
#include "../helper/string.h"

#include <algorithm>


void NicosData::FilterComments(LoadTxt::t_mapComm& mapComm)
{
	LoadTxt::t_mapComm::iterator iter;
	for(iter=mapComm.begin(); iter!=mapComm.end();)
	{
		const std::string& strKey = (*iter).first;

		if(begins_with(strKey, std::string("##")))
		{
			mapComm.erase(iter);
			iter = mapComm.begin();
			continue;
		}

		++iter;
	}
}

NicosData::NicosData(const LoadTxt& data) : m_data(data), m_bOk(0)
{
	const std::vector<std::string>& strAux = m_data.GetAuxStrings();

	if(strAux.size() >= 1)
		::get_tokens<std::string>(strAux[0], std::string(" \t"), m_vecColNames);
	if(strAux.size() >= 2)
		::get_tokens<std::string>(strAux[1], std::string(" \t"), m_vecColUnits);

	//for(auto str : m_vecColNames) std::cout << str << std::endl;
	//for(auto str : m_vecColUnits) std::cout << str << std::endl;

	LoadTxt& dat = const_cast<LoadTxt&>(data);
	FilterComments(dat.GetCommMap());

	m_bOk = 1;
}

NicosData::~NicosData()
{	}


const std::string& NicosData::GetColName(unsigned int iIdx) const
{
	static const std::string strNull = "";

	if(iIdx < m_vecColNames.size())
		return m_vecColNames[iIdx];

	return strNull;
}

const std::string& NicosData::GetColUnit(unsigned int iIdx) const
{
	static const std::string strNull = "";

	if(iIdx < m_vecColUnits.size())
		return m_vecColUnits[iIdx];

	return strNull;
}

int NicosData::GetColIdx(const std::string& strName) const
{
	std::vector<std::string>::const_iterator iter =
			std::find(m_vecColNames.begin(), m_vecColNames.end(), strName);

	if(iter == m_vecColNames.end())
		return -1;

	return iter-m_vecColNames.begin();
}

std::string NicosData::TryFindScanVar(LoadTxt::t_mapComm& mapComm) const
{
	LoadTxt::t_mapComm::iterator iter = mapComm.find("info");
	if(iter == mapComm.end())
		return "";
	if(iter->second.size() < 1)
		return "";

	// e.g. cscan(sgy, 0, 0.4, 10, 2)
	std::string strScan = iter->second[0];
	std::size_t iPos0 = strScan.find('(');
	std::size_t iPos1 = strScan.find(',');
	if(iPos0==std::string::npos || iPos1==std::string::npos || iPos1<iPos0)
		return "";

	std::string strVar = strScan.substr(iPos0+1,iPos1-iPos0-1);
	::trim(strVar);
	//std::cout << "Scan-Var: " << strVar << std::endl;

	return strVar;
}
