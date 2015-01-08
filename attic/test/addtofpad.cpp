#include <iostream>
#include <fstream>
#include <vector>
#include "../helper/file.h"

int main(int argc, char** argv)
{
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

	for(int iInFile=0; iInFile<iNumInFiles; ++iInFile)
	{
		const char* pcFile = argv[iInFile+1];
		std::ifstream ifstr(pcFile, std::ios::binary);
		if(!ifstr.is_open())
		{
			std::cerr << "Error: Could not open \"" << pcFile << "\"."
					 << " Skipping."
					 << std::endl;
			continue;
		}

		unsigned int iInSize = get_file_size(ifstr) / sizeof(int);
		if(!bHasSize)
		{
			vecSum.resize(iInSize);
			for(unsigned int i=0; i<iInSize; ++i)
				vecSum[i] = 0;
			bHasSize = 1;
		}

		for(unsigned int i=0; i<iInSize; ++i)
		{
			unsigned int iVal = 0;
			ifstr.read((char*)&iVal, sizeof(iVal));
			vecSum[i] += iVal;
		}

		ifstr.close();
	}


	std::ofstream ofstr(argv[argc-1], std::ios::binary);
	for(unsigned int iSize=0; iSize<vecSum.size(); ++iSize)
	{
		ofstr.write((char*)&vecSum[iSize], sizeof(int));
	}

	ofstr.close();

	return 0;
}
