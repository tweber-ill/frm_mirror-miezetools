/*
 * loading of nicos data files
 * @author tweber
 * @data 25-04-2013
 */

#include "loadnicos.h"
#include "../helper/string.h"

#include <algorithm>

NicosData::NicosData(const LoadTxt& data) : m_data(data), m_bOk(0)
{
	const std::vector<std::string>& strAux = m_data.GetAuxStrings();

	if(strAux.size() >= 1)
		::get_tokens<std::string>(strAux[0], " \t", m_vecColNames);
	if(strAux.size() >= 2)
		::get_tokens<std::string>(strAux[1], " \t", m_vecColUnits);

	//for(auto str : m_vecColNames) std::cout << str << std::endl;
	//for(auto str : m_vecColUnits) std::cout << str << std::endl;

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
