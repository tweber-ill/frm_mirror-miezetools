/**
 * pad/tof files
 *
 * @author Tobias Weber
 * @date September 2012, 19-mar-2013
 */

#ifndef __PADTOF__
#define __PADTOF__

#include <QtCore/QFile>
#include <vector>
#include <map>
#include "../tlibs/string/string.h"
#include "../helper/string_map.h"


class PadFile
{
protected:
	QFile m_file;
	StringMap m_params;

	void LoadParams();

public:
	PadFile(const char* pcFile);
	virtual ~PadFile();

	static unsigned int GetWidth();
	static unsigned int GetHeight();

	bool IsOpen() const;
	const unsigned int* GetData();
	void ReleaseData(const unsigned int *pv);

	const StringMap& GetParamMap() { return m_params; }
};


class TofFile
{
protected:
	QFile m_file;
	StringMap m_params;

	void LoadParams();

public:
	TofFile(const char* pcFile);
	virtual ~TofFile();

	static unsigned int GetWidth();
	static unsigned int GetHeight();
	static unsigned int GetFoilCnt();
	static unsigned int GetTcCnt();
	static unsigned int GetImgCnt();
	static std::vector<unsigned int> GetStartIndices();

	bool IsOpen() const;
	const unsigned int* GetData(unsigned int iFoil);
	void ReleaseData(const unsigned int *pv);

	const StringMap& GetParamMap() { return m_params; }
};

#endif
