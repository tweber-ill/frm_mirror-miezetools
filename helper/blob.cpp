/*
 * binary large object
 * @author tweber
 * @date 26-jun-2013
 */

#include "blob.h"
#include <iostream>


Blob::Blob(const char* pcFile) : m_file(QString(pcFile))
{
	m_file.open(QIODevice::ReadOnly);
}

Blob::~Blob()
{
	m_file.close();
}


bool Blob::IsOpen() const
{
	return m_file.isOpen();
}

void* Blob::map(qint64 iStart, qint64 iLen)
{
	qint64 iSize = m_file.size();
	if(iStart+iLen > iSize)
	{
		std::cerr << "Error: Tried to map beyond size." << std::endl;
		return 0;
	}

	return m_file.map(iStart, iLen);
}

void Blob::unmap(void *pv)
{
	m_file.unmap((uchar*)pv);
}