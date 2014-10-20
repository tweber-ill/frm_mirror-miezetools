/*
 * string helper
 * @author tweber
 * @date 06-mar-2013
 */

#ifndef __MIEZE_STRINGMAP__
#define __MIEZE_STRINGMAP__

#include <vector>
#include <map>
#include <string>

class StringMap;
class StringMap
{
	public:
		typedef std::map<std::string, std::string> t_map;

	protected:
		t_map m_map;
		std::string m_strKeyValSeparator;
		std::string m_strComment;

	public:
		StringMap(const char* pcKeyValSep=":", const char* pcComment="#");
		virtual ~StringMap();

		void ParseString(const std::string& strConf);

		void SetMap(const t_map& map) { m_map = map; }
		const t_map& GetMap() const { return m_map; }
		t_map& GetMap() { return m_map; }

		bool Serialize(std::ostream& ostrSer) const;
		bool Deserialize(const void* pvMem, unsigned int iLen);

		bool HasKey(const std::string& str) const;
		std::string& operator[](const std::string& str);
		const std::string& operator[](const std::string& str) const;

		const std::vector<std::string> GetKeys() const;

		void Trim();
		void MergeFrom(const std::vector<const StringMap*>& vecMaps);
};

extern std::ostream& operator<<(std::ostream& ostr, const StringMap& mapStr);


template<typename T>
std::map<T, T> vecmap_to_map(const std::map<T, std::vector<T> >& themap)
{
	std::map<T,T> singleMap;

	for(const auto& pair : themap)
	{
		//const T& key = pair.first;
		const std::vector<T>& vect = pair.second;

		T tval;
		if(vect.size() > 0)
			tval = vect[0];

		singleMap.insert(std::pair<T, T>(pair.first, tval));
	}

	return singleMap;
}

#endif
