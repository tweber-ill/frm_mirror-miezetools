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
#include <iostream>


PadFile::PadFile(const char* pcFile)
		: m_file(QString(pcFile)),
		  m_params(":", "#")
{
	m_file.open(QIODevice::ReadOnly);

	if(IsOpen())
		LoadParams();
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

void PadFile::LoadParams()
{
	qint64 iDataLen = qint64(GetWidth()*GetHeight()*sizeof(int));
	qint64 iFileLen = m_file.size();
	qint64 iParamLen = iFileLen-iDataLen;

	if(iParamLen > 0)
	{
		const char* pcParams = (char*)m_file.map(iDataLen, iParamLen);

		std::string strParams;
		strParams.assign(pcParams, std::size_t(iParamLen));

		m_params.ParseString(strParams);

		m_file.unmap((uchar*)pcParams);
	}
}

const unsigned int* PadFile::GetData()
{
	const unsigned int iW = GetWidth();
	const unsigned int iH = GetHeight();

	qint64 iLen = qint64(iW*iH*sizeof(int));

	qint64 iSize = m_file.size();
	if(iLen > iSize)
	{
		std::cerr << "Error: Tried to map beyond size." << std::endl;
		return 0;
	}

	unsigned int *pDat = (unsigned int*)m_file.map(qint64(0), iLen);
	return pDat;
}

void PadFile::ReleaseData(const unsigned int *pv)
{
	m_file.unmap((uchar*)pv);
}



TofFile::TofFile(const char* pcFile)
		: m_file(QString(pcFile)),
		  m_params(":", "#")
{
	m_file.open(QIODevice::ReadOnly);

	if(IsOpen())
		LoadParams();
}

TofFile::~TofFile()
{}

void TofFile::LoadParams()
{
	qint64 iDataLen = qint64(GetWidth()*GetHeight()*GetImgCnt()*sizeof(int));
	qint64 iFileLen = m_file.size();
	qint64 iParamLen = iFileLen-iDataLen;

	if(iParamLen > 0)
	{
		const char* pcParams = (char*)m_file.map(iDataLen, iParamLen);

		std::string strParams;
		strParams.assign(pcParams, std::size_t(iParamLen));
		//std::cout << strParams << std::endl;

		m_params.ParseString(strParams);
		m_file.unmap((uchar*)pcParams);
	}
}

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

// total images, including non-used ones between half-spaces
unsigned int TofFile::GetImgCnt()
{
	std::vector<unsigned int> vecStartIndices = GetStartIndices();
	unsigned int iLastIdx = *vecStartIndices.rbegin();

	return iLastIdx+GetTcCnt();
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

	qint64 iStart = qint64(iStartIdx*iW*iH*sizeof(int));
	qint64 iLen = qint64(iW*iH*iTcCnt*sizeof(int));

	qint64 iSize = m_file.size();
	if(iStart+iLen > iSize)
	{
		std::cerr << "Error: Tried to map beyond size." << std::endl;
		return 0;
	}

	unsigned int *pDat = (unsigned int*)m_file.map(iStart, iLen);
	return pDat;
}

void TofFile::ReleaseData(const unsigned int *pv)
{
	m_file.unmap((uchar*)pv);
}
