// gcc -o mergetof mergetof.cpp ../helper/file.cpp ../helper/string.cpp ../helper/rand.cpp ../helper/comp.cpp -lstdc++ -lQtCore -lboost_iostreams -std=c++11

#include <iostream>
#include <fstream>
#include <vector>
#include "../helper/file.h"
#include "../helper/string.h"
#include "../helper/comp.h"


int main(int argc, char** argv)
{
	const unsigned int iTOFSize = 128*128*128;

	if(argc<3)
	{
		std::cerr << "Usage: " << argv[0]
		                       << " in_file1 in_file2 ... out_file"
		                       << std::endl;
		return -1;
	}


	int iNumInFiles = argc-2;
	std::vector<unsigned int> vecSum;
	bool bHasSize = 0;


	char *pcStrings = 0;
	unsigned int iRemaining = 0;

	for(int iInFile=0; iInFile<iNumInFiles; ++iInFile)
	{
		TmpFile *pTmp = 0;

		const char* pcFile = argv[iInFile+1];
		std::string strFile = pcFile;
		
		std::cout << strFile << "...";

		std::string strExt = get_fileext(pcFile);
		if(strExt == "gz" || strExt == "bz2" || strExt == "xz")
		{
			pTmp = new TmpFile();
			pTmp->open();

			strFile = pTmp->GetFileName();
			decomp_file_to_file(pcFile, strFile.c_str());
			pcFile = strFile.c_str();
		}


		std::ifstream ifstr(pcFile, std::ios::binary);
		if(!ifstr.is_open())
		{
			std::cerr << "Error: Could not open \"" << pcFile << "\"."
					 << " Skipping."
					 << std::endl;
			continue;
		}

		if(!bHasSize)
		{
			vecSum.resize(iTOFSize);
			for(unsigned int i=0; i<iTOFSize; ++i)
				vecSum[i] = 0;
			bHasSize = 1;
		}

		unsigned int iTotalCnts = 0;
		for(unsigned int i=0; i<iTOFSize; ++i)
		{
			unsigned int iVal = 0;
			ifstr.read((char*)&iVal, sizeof(iVal));
			vecSum[i] += iVal;

			iTotalCnts += iVal;
		}

		std::cout << " " << iTotalCnts << " counts." << std::endl;


		if(!pcStrings)
		{
			iRemaining = get_file_size(ifstr) - iTOFSize*sizeof(int);
			pcStrings = new char[iRemaining];
			ifstr.read(pcStrings, iRemaining);

			//std::cout << pcStrings << std::endl;
		}

		if(pTmp) delete pTmp;
		ifstr.close();
	}


	std::ofstream ofstr(argv[argc-1], std::ios::binary);
	for(unsigned int iSize=0; iSize<vecSum.size(); ++iSize)
	{
		ofstr.write((char*)&vecSum[iSize], sizeof(int));
	}

	if(pcStrings)
	{
		ofstr.write(pcStrings, iRemaining);
		delete[] pcStrings;
	}

	ofstr.close();

	return 0;
}
