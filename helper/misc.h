/**
 * misc helper
 * @author Tobias Weber <tobias.weber@tum.de>
 * @date 07-mar-2013
 * @license GPLv3
 */

#ifndef __MIEZE_MISC_H__
#define __MIEZE_MISC_H__

#include "tlibs/string/string.h"

template<class T> class autodeleter
{
protected:
	T *m_t;
	bool m_bIsArray;

public:
	autodeleter(T* t, bool bIsArray=false) : m_t(t), m_bIsArray(bIsArray)
	{}

	~autodeleter()
	{
		if(m_t)
		{
			if(m_bIsArray)
				delete[] m_t;
			else
				delete m_t;
			m_t = 0;
		}
	}
};


// e.g. str = "123.4 +- 0.5"
template<typename T=double, class t_str=std::string>
void get_val_and_err(const t_str& str, T& val, T& err)
{
	// "+-", inelegant...
	t_str strPlusMinus;
	strPlusMinus.resize(2);
	strPlusMinus[0] = '+';
	strPlusMinus[1] = '-';

	std::vector<T> vec;
	tl::get_tokens<T, t_str>(str, strPlusMinus, vec);

	if(vec.size() >= 1)
		val = vec[0];
	if(vec.size() >= 2)
		err = vec[1];
}


#endif
