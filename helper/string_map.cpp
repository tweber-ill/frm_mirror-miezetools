/**
 * string helper
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 06-mar-2013
 * @license GPLv3
 */

#include "string_map.h"
#include "tlibs/string/string.h"
#include "tlibs/helper/misc.h"
#include "tlibs/log/log.h"

#ifndef NO_COMP
	#include "tlibs/file/comp.h"
#endif

StringMap::StringMap(const char* pcKeyValSep, const char* pcComment)
{
	if(pcComment)
		m_strComment = pcComment;
	if(pcKeyValSep)
		m_strKeyValSeparator = pcKeyValSep;
}

StringMap::~StringMap()
{
}

const std::vector<std::string> StringMap::GetKeys() const
{
	std::vector<std::string> vecKeys;
	vecKeys.reserve(m_map.size());

	for(const t_map::value_type& val : m_map)
		vecKeys.push_back(val.first);

	return vecKeys;
}

bool StringMap::HasKey(const std::string& str) const
{
	return (m_map.find(str) != m_map.end());
}

std::string& StringMap::operator[](const std::string& str)
{
	t_map::iterator iter = m_map.find(str);
	if(iter == m_map.end())
		return m_map.insert(t_map::value_type(str, "")).first->second;

	return iter->second;
}

const std::string& StringMap::operator[](const std::string& str) const
{
	static std::string strDummy;

	t_map::const_iterator iter = m_map.find(str);
	if(iter != m_map.end())
		return iter->second;

	//std::cerr << "Warning: Returning dummy object." << std::endl;
	return strDummy;
}

void StringMap::Trim()
{
	for(t_map::value_type& pair : m_map)
	{
		tl::trim(pair.second);

		if(pair.first == "" || pair.second == "")
			m_map.erase(pair.first);
	}
}

void StringMap::MergeFrom(const std::vector<const StringMap*>& vecMaps)
{
	m_map.clear();

	for(const StringMap* pMap : vecMaps)
		tl::merge_map<std::string, std::string>(m_map, pMap->GetMap());
}

void StringMap::ParseString(const std::string& strConf)
{
	std::istringstream istr(strConf);
	while(!istr.eof())
	{
		std::string strLine;
		std::getline(istr, strLine);

		tl::trim(strLine);
		if(strLine.length()==0)
			continue;

		if(m_strComment.length()>0 && strLine.substr(0,m_strComment.length())==m_strComment)
			continue;

		std::pair<std::string, std::string> pairStr = tl::split_first(strLine, m_strKeyValSeparator, 1);
		if(pairStr.first.length()==0 && pairStr.second.length()==0)
			continue;

		bool bInserted = m_map.insert(pairStr).second;
		if(!bInserted)
		{
			tl::log_warn("Key \"", pairStr.first, "\" already exists in map.");
		}

		//std::cout << "key: \"" << pairStr.first <<"\" value:\"" << pairStr.second << "\"" << std::endl;
	}
}

#ifndef NO_COMP
bool StringMap::Serialize(std::ostream& ostrSer) const
{
	unsigned int iLen = 0;
	for(const auto& pair : m_map)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		iLen += strKey.length()+1;
		iLen += strVal.length()+1;
	}

	char *pcMem = new char[iLen];
	memset(pcMem, 0, iLen);
	unsigned int iCurIdx=0;

	for(const auto& pair : m_map)
	{
		const std::string& strKey = pair.first;
		const std::string& strVal = pair.second;

		memcpy(pcMem+iCurIdx, strKey.c_str(), strKey.length());
		iCurIdx += strKey.length()+1;
		memcpy(pcMem+iCurIdx, strVal.c_str(), strVal.length());
		iCurIdx += strVal.length()+1;
	}

	bool bOk = tl::comp_mem_to_stream<char>(pcMem, iLen, ostrSer, tl::Compressor::BZ2);
	delete[] pcMem;

	return bOk;
}


static inline std::vector<std::string> split0(const char* pcMem, unsigned int iLen)
{
	std::vector<std::string> vecStr;
	if(iLen==0) return vecStr;

	vecStr.push_back(std::string(pcMem));
	//std::cout << pcMem << std::endl;

	for(unsigned int iIdx=0; iIdx<iLen-1; ++iIdx)
	{
		if(pcMem[iIdx] == 0)
		{
			std::string str = pcMem+iIdx+1;
			tl::trim(str);
			vecStr.push_back(str);

			//std::cout << str << std::endl;
		}
	}

	return vecStr;
}

bool StringMap::Deserialize(const void* pvMem, unsigned int iLen)
{
	char *pcUncomp = 0;
	std::size_t iLenUncomp = 0;
	if(!tl::decomp_mem_to_mem<char>((char*)pvMem, iLen, pcUncomp, iLenUncomp))
		return false;

	std::vector<std::string> vecStrings = split0(pcUncomp, iLenUncomp);
	if(pcUncomp) delete[] pcUncomp;

	if(vecStrings.size()%2 != 0)
	{
		tl::log_err("Uneven number of strings in key/value map.");
		return false;
	}

	for(unsigned int iIdx=0; iIdx<vecStrings.size(); iIdx+=2)
	{
		const std::string& strKey = vecStrings[iIdx];
		const std::string& strVal = vecStrings[iIdx+1];

		m_map.insert(std::pair<std::string,std::string>(strKey, strVal));
	}

	Trim();
	return true;
}
#else
bool StringMap::Serialize(std::ostream& ostrSer) const
{
	log_err("Serialize not linked.");
	return false;
}

bool StringMap::Deserialize(const void* pvMem, unsigned int iLen)
{
	log_err("Deserialize not linked.");
	return false;
}
#endif

std::ostream& operator<<(std::ostream& ostr, const StringMap& mapStr)
{
	const StringMap::t_map& map = mapStr.GetMap();
	for(const StringMap::t_map::value_type& pair : map)
		ostr <<  pair.first << " = " << pair.second << std::endl;
	return ostr;
}
