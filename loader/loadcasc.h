/**
 * pad/tof files
 *
 * @author Tobias Weber
 * @date September 2012
 */

#ifndef __PADTOF__
#define __PADTOF__

#include <QtCore/QFile>
#include <vector>

class PadFile
{
protected:
	QFile m_file;

public:
	PadFile(const char* pcFile);
	virtual ~PadFile();

	static unsigned int GetWidth();
	static unsigned int GetHeight();

	bool IsOpen() const;
	const unsigned int* GetData();
};


class TofFile
{
protected:
	QFile m_file;

public:
	TofFile(const char* pcFile);
	virtual ~TofFile();

	static unsigned int GetWidth();
	static unsigned int GetHeight();
	static unsigned int GetFoilCnt();
	static unsigned int GetTcCnt();
	static std::vector<unsigned int> GetStartIndices();

	bool IsOpen() const;
	const unsigned int* GetData(unsigned int iFoil);
};

#endif
