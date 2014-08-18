// clang -O2 -o colsort colsort.cpp ../helper/string.cpp -lstdc++ -std=c++11 

#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "../helper/string.h"



struct Sorter
{
	const std::vector<unsigned int>* pvecSortIndices = 0;
	static unsigned int iSortSteps;

	bool operator()(const std::vector<double>& vec1, const std::vector<double>& vec2) /*const*/
	{
		++iSortSteps;

		const std::vector<unsigned int>& vecIdx = *pvecSortIndices;
		for(unsigned int iIdx=0; iIdx<vecIdx.size(); ++iIdx)
		{
			unsigned int iCurIdx = vecIdx[iIdx];
			if(vec1[iCurIdx] < vec2[iCurIdx])
				return 1;
			else if(vec1[iCurIdx] == vec2[iCurIdx])
				continue;
			else if(vec1[iCurIdx] > vec2[iCurIdx])
				return false;
		}

		return 0;
	}
};

unsigned int Sorter::iSortSteps = 0;



void sortdata(std::vector<std::vector<double> >& vecData, const std::vector<unsigned int>& vecSortIndices)
{
	/*bool (*sortfkt)(const std::vector<double>&, const std::vector<double>&)
		= [](const std::vector<double>& vec1, const std::vector<double>& vec2) -> bool
		{
			return vec1[1] < vec2[1];
		};*/

	Sorter sortfkt;
	sortfkt.pvecSortIndices = &vecSortIndices;

	std::sort(vecData.begin(), vecData.end(), sortfkt);
}



int main(int argc, char **argv)
{
	std::string strInFile;
	std::string strOutFile;

	std::vector<unsigned int> vecSortIndices;

	int iOpt=0;
	while((iOpt=getopt(argc, argv, "i:o:s:")) > 0)
	{
		switch(iOpt)
		{
			case 'i':
				strInFile = optarg;
				break;
			case 'o':
				strOutFile = optarg;
				break;
			case 's':
			{
				std::string strSort = optarg;
				get_tokens<unsigned int, std::string>(strSort, " ", vecSortIndices);
				break;
			}
		}
	}

	//trim(strInFile);
	//trim(strOutFile);

	//if(strOutFile == "")
	//	strOutFile = strInFile;

	std::cout << "Input file: " << strInFile << "\n";
	std::cout << "Output file: " << strOutFile << "\n";
	std::cout << "Sort indices: ";
	for(unsigned int iIdx=0; iIdx<vecSortIndices.size(); ++iIdx)
		std::cout << vecSortIndices[iIdx] << " ";
	std::cout << std::endl;

	if(vecSortIndices.size() == 0)
	{
		std::cerr << "No sort indices given." << std::endl;
		return -1;
	}

	if(strInFile.length() == 0)
	{
		std::cerr << "No input file given." << std::endl;
		return -1;
	}

	if(strOutFile.length() == 0)
	{
		std::cerr << "No output file given." << std::endl;
		return -1;
	}



	std::cout << "\n";
	std::cout << "Reading \"" << strInFile << "\"." << std::endl;
	std::ifstream infile(strInFile);
	if(!infile.is_open())
	{
		std::cerr << "Cannot open file for input." << std::endl;
		return -1;
	}

	std::string strHeader;
	unsigned int iNumCols = 0;

	std::vector<std::vector<double>> vecData;

	while(!infile.eof())
	{
		std::string strLine;
		std::getline(infile, strLine);
		trim(strLine);

		if(strLine.length() == 0)
			continue;
		if(!std::isdigit(strLine[0]) && strLine[0]!='.')
		{
			strHeader += strLine + "\n";
			continue;
		}

		std::vector<double> vecToks;
		get_tokens<double, std::string>(strLine, "\t ", vecToks);

		if(iNumCols == 0)
		{
			iNumCols = vecToks.size();
			std::cout << "Number of data columns: " << iNumCols << std::endl;
		}

		if(vecToks.size() != iNumCols)
		{
			std::cerr << "Invalid number of columns." << std::endl;
			continue;
		}

		std::vector<double> vecRow;
		for(unsigned int iCol=0; iCol<iNumCols; ++iCol)
			vecRow.push_back(vecToks[iCol]);
		vecData.push_back(vecRow);
	}

	std::cout << "Number of data rows: " << vecData.size() << std::endl;



	std::cout << "\n";
	std::cout << "Sorting." << std::endl;
	sortdata(vecData, vecSortIndices);
	std::cout << "Iterations needed for sorting: " << Sorter::iSortSteps << std::endl;



	std::cout << "\n";
	std::cout << "Writing \"" << strOutFile << "\"." << std::endl;
	std::ofstream outfile(strOutFile);
	if(!outfile.is_open())
	{
		std::cerr << "Cannot open file for output." << std::endl;
		return -1;
	}
	outfile << strHeader;
	for(unsigned int iRow=0; iRow<vecData.size(); ++iRow)
	{
		for(unsigned int iCol=0; iCol<iNumCols; ++iCol)
			outfile << vecData[iRow][iCol] << "    ";
		outfile << "\n";
	}


	std::cout << "Done." << std::endl;
	return 0;
}
