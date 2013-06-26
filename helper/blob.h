/*
 * binary large object
 * @author tweber
 * @date 26-jun-2013
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
		void copy(qint64 iStart, qint64 iLen, Iter begin)
		{
			T* p = (T*)map(iStart, iLen);

			for(qint64 i=0; i<iLen; ++i)
			{
				*begin = T(p[i]);
				begin++;
			}

			unmap((void*)p);
		}
};

#endif
