/*
 * mieze-tool
 * data export
 * @author tweber
 * @date 17-jun-2013
 */

#include "export.h"
#include "../plot/plot.h"
#include "../plot/plot2d.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>

static std::string get_py_linestyle(PlotType plttype, unsigned int iLine)
{
	static const std::string strStyles[] = {"o", "s", "^", "*"};

	if(plttype == PLOT_FKT)
		return "-";

	if(plttype == PLOT_DATA)
		return strStyles[iLine % (sizeof(strStyles)/sizeof(strStyles[0]))];
}

bool export_py(const char* pcFile, const SubWindowBase *pSWB)
{
	if(!pSWB) return false;

	std::ofstream ofstr(pcFile);
	if(!ofstr.is_open())
	{
		std::cerr << "Error: Cannot open file \"" << pcFile << "\"." << std::endl;
		return false;
	}

	ofstr << "import matplotlib.pyplot as plt\n\n";

	double dXLim[2];
	double dYLim[2];

	dYLim[0] = dXLim[0] = std::numeric_limits<double>::max();
	dYLim[1] = dXLim[1] = -dXLim[0];

	pSWB = const_cast<SubWindowBase*>(pSWB)->GetActualWidget();
	if(pSWB->GetType() == PLOT_1D)
	{
		const Plot* plt = (Plot*)pSWB;
		for(unsigned int iLine=0; iLine<plt->GetDataCount(); ++iLine)
		{
			const PlotObj& obj = plt->GetData(iLine);
			const Data1& dat = obj.dat;

			std::string strLineStyle = get_py_linestyle(obj.plttype, iLine);

			ofstr << "x_" << iLine << " = [";
			for(unsigned int iLen=0; iLen<dat.GetLength(); ++iLen)
				ofstr << dat.GetX(iLen) << ", ";
			ofstr << "]\n";

			ofstr << "y_" << iLine << " = [";
			for(unsigned int iLen=0; iLen<dat.GetLength(); ++iLen)
				ofstr << dat.GetY(iLen) << ", ";
			ofstr << "]\n";

			if(obj.plttype == PLOT_DATA)
			{
				ofstr << "xerr_" << iLine << " = [";
				for(unsigned int iLen=0; iLen<dat.GetLength(); ++iLen)
					ofstr << dat.GetXErr(iLen) << ", ";
				ofstr << "]\n";

				ofstr << "yerr_" << iLine << " = [";
				for(unsigned int iLen=0; iLen<dat.GetLength(); ++iLen)
					ofstr << dat.GetYErr(iLen) << ", ";
				ofstr << "]\n";
				ofstr << "\n";
			}

			if(obj.plttype == PLOT_DATA)
			{
				ofstr << "plt.errorbar(x_" << iLine
						<< ", " << "y_" << iLine
						<< ", " << "xerr=xerr_" << iLine
						<< ", " << "yerr=yerr_" << iLine
						<< ", " << "fmt=\"" << strLineStyle << "\""
						<< ")\n\n";
			}
			else if(obj.plttype == PLOT_FKT)
			{
				ofstr << "plt.plot(x_" << iLine
						<< ", " << "y_" << iLine
						<< ", " << "ls=\"" << strLineStyle << "\""
						<< ", " << "lw=1.4"
						<< ")\n\n";
			}

			ofstr << "plt.xlabel(\"" << plt->GetXLabel() << "\")\n";
			ofstr << "plt.ylabel(\"" << plt->GetYLabel() << "\")\n";

			double dXMin, dXMax, dXErrMin, dXErrMax;
			double dYMin, dYMax, dYErrMin, dYErrMax;
			dat.GetXMinMax(dXMin, dXMax);
			dat.GetXErrMinMax(dXErrMin, dXErrMax);
			dat.GetYMinMax(dYMin, dYMax);
			dat.GetYErrMinMax(dYErrMin, dYErrMax);

			dXLim[0] = std::min(dXLim[0], dXMin-dXErrMax);
			dXLim[1] = std::max(dXLim[1], dXMax+dXErrMax);
			dYLim[0] = std::min(dYLim[0], dYMin-dYErrMax);
			dYLim[1] = std::max(dYLim[1], dYMax+dYErrMax);

			ofstr << "\n\n";
		}

		ofstr << "plt.xlim(" << dXLim[0] << ", " << dXLim[1] << ")\n";
		ofstr << "plt.ylim(" << dYLim[0] << ", " << dYLim[1] << ")\n";

		ofstr << "plt.grid(True)\n";
		ofstr << "plt.show()\n";
	}
	else if(pSWB->GetType() == PLOT_2D)
	{
		const Plot2d* plt = (Plot2d*)pSWB;



	}
	else
	{
		std::cerr << "Error: Data type not (yet) supported." << std::endl;
		return false;
	}

	return true;
}
