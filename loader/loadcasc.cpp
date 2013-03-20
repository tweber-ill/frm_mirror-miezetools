/**
 * pad/tof files
 *
 * @author Tobias Weber
 * @date September 2012, 19-mar-2013
 */

#include "loadcasc.h"
#include "../settings.h"

#include <QtCore/QVector>
#include <QtCore/QList>


PadFile::PadFile(const char* pcFile) : m_file(QString(pcFile))
{
	m_file.open(QIODevice::ReadOnly);
}

PadFile::~PadFile()
{}

bool PadFile::IsOpen() const { return m_file.isOpen(); }

unsigned int PadFile::GetWidth()
{
	return Settings::Get<unsigned int>("casc/x_res");
}

unsigned int PadFile::GetHeight()
{
	return Settings::Get<unsigned int>("casc/y_res");
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
	return Settings::Get<unsigned int>("casc/foil_cnt");
}

unsigned int TofFile::GetTcCnt()
{
	return Settings::Get<unsigned int>("casc/tc_cnt");
}

std::vector<unsigned int> TofFile::GetStartIndices()
{
	QList<QVariant> lst = Settings::Get<QList<QVariant> >("casc/foil_idx");

	std::vector<unsigned int> vec;
	vec.resize(lst.size());
	for(int i=0; i<lst.size(); ++i)
		vec[i] = lst[i].toUInt();

	return vec;
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
