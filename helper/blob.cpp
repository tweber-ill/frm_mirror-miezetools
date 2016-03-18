/*
 * binary large object
 * @author tweber
 * @date 26-jun-2013
 */

#include "blob.h"
#include "../tlibs/log/log.h"
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
		tl::log_err("Tried to map beyond size.");
		return 0;
	}

	return m_file.map(iStart, iLen);
}

void Blob::unmap(void *pv)
{
	m_file.unmap((uchar*)pv);
}


void Blob::memcpy(qint64 iStart, qint64 iLen, void* pvStart)
{
	void* p = map(iStart, iLen);
	::memcpy(pvStart, p, iLen);
	unmap(p);
}
