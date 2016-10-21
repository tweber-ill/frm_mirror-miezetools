/*
 * binary large object
 * @author tweber
 * @date 26-jun-2013
 * @license GPLv3
 */

#ifndef __MIEZE_BLOB__
#define __MIEZE_BLOB__

#include <QtCore/QFile>

class Blob
{
	protected:
		QFile m_file;

	public:
		Blob(const char* pcFile);
		virtual ~Blob();

		bool IsOpen() const;

		void* map(qint64 iStart, qint64 iLen);
		void unmap(void* pv);

		template<typename T, class Iter>
		void copy(qint64 iStart, qint64 iLenInT, Iter begin)
		{
			T* p = (T*)map(iStart, iLenInT*sizeof(T));

			for(qint64 i=0; i<iLenInT; ++i)
			{
				*begin = T(p[i]);
				begin++;
			}

			unmap((void*)p);
		}

		template<typename T>
		void vec_push_back(qint64 iStart, qint64 iLenInT, std::vector<T>& vec)
		{
			T* p = (T*)map(iStart, iLenInT*sizeof(T));

			for(qint64 i=0; i<iLenInT; ++i)
				vec.push_back(T(p[i]));

			unmap((void*)p);
		}

		void memcpy(qint64 iStart, qint64 iLen, void* pvStart);
};

#endif
