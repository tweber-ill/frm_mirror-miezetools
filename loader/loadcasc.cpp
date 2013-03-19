/**
 * pad/tof files
 *
 * @author Tobias Weber
 * @date September 2012, 19-mar-2013
 */

#include "loadcasc.h"

PadFile::PadFile(const char* pcFile) : m_file(QString(pcFile))
{
	m_file.open(QIODevice::ReadOnly);
}

PadFile::~PadFile()
{}

bool PadFile::IsOpen() const { return m_file.isOpen(); }

unsigned int PadFile::GetWidth()
{
	return 128;
}

unsigned int PadFile::GetHeight()
{
	return 128;
}

const unsigned int* PadFile::GetData()
{
	const unsigned int iW = GetWidth();
	const unsigned int iH = GetHeight();

	unsigned int *pDat = (unsigned int*)m_file.map(qint64(0), qint64(iW*iH*sizeof(int)));
	return pDat;
}

void PadFile::ReleaseData(const unsigned int *pv)
{
	m_file.unmap((uchar*)pv);
}



TofFile::TofFile(const char* pcFile) : m_file(QString(pcFile))
{
	m_file.open(QIODevice::ReadOnly);
}

TofFile::~TofFile()
{}

unsigned int TofFile::GetWidth()
{ return PadFile::GetWidth(); }

unsigned int TofFile::GetHeight()
{ return PadFile::GetHeight(); }

unsigned int TofFile::GetFoilCnt()
{
	return 6;
}

unsigned int TofFile::GetTcCnt()
{
	return 16;
}

std::vector<unsigned int> TofFile::GetStartIndices()
{
	std::vector<unsigned int> vecStartIndices = {0, 16, 32, 64, 80, 96};
	return vecStartIndices;
}

bool TofFile::IsOpen() const { return m_file.isOpen(); }

const unsigned int* TofFile::GetData(unsigned int iFoil)
{
	const unsigned int iW = GetWidth();
	const unsigned int iH = GetHeight();
	const unsigned int iFCnt = GetFoilCnt();
	const unsigned int iTcCnt = GetTcCnt();

	if(iFoil >= iFCnt)
		return 0;

	std::vector<unsigned int> vecStartIndices = GetStartIndices();
	const unsigned int iStartIdx = vecStartIndices[iFoil];

	unsigned int *pDat = (unsigned int*)m_file.map(qint64(iStartIdx*iW*iH),
														qint64(iW*iH*iTcCnt*sizeof(int)));
	return pDat;
}

void TofFile::ReleaseData(const unsigned int *pv)
{
	m_file.unmap((uchar*)pv);
}
